use serial2::SerialPort;
use serialport::SerialPortType;
use std::{
    mem::zeroed,
    net::TcpListener,
    sync::mpsc::{self, Sender},
    thread,
    time::Duration,
};
use tauri::{command, Emitter, Manager};

/// The presented port for communication with the Arduino.
static mut PORT: SerialPort = unsafe { zeroed() };
/// The presented listener for local communication.
static mut SRV: TcpListener = unsafe { zeroed() };

/// Sends data to the Arduino.
#[command]
fn send(data: &str) {
    let m = format!("{data}\n");
    unsafe {
        PORT.write(m.as_bytes()).unwrap();
    }
}

/// Listens for USB changes.
fn listen_usb() {
    #[cfg(windows)]
    {
        let com_con = wmi::COMLibrary::new().unwrap();
        let wmi_con = wmi::WMIConnection::new(com_con).unwrap();
        let _events = wmi_con
            .raw_notification::<std::collections::HashMap<String, wmi::Variant>>(
                "SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2",
            )
            .unwrap();
    }
    #[cfg(unix)]
    {
        unsafe {
            let udev = udev::udev_new();
            if udev.is_null() {
                eprintln!("Failed to create udev object");
                return;
            }

            let monitor = udev::udev_monitor_new_from_netlink(
                udev,
                CStr::from_bytes_with_nul_unchecked(b"udev\0").as_ptr(),
            );
            if monitor.is_null() {
                eprintln!("Failed to create udev monitor");
                udev::udev_unref(udev);
                return;
            }

            if udev::udev_monitor_enable_receiving(monitor) < 0 {
                eprintln!("Failed to enable receiving udev events");
                udev::udev_monitor_unref(monitor);
                udev::udev_unref(udev);
                return;
            }

            let fd = udev::udev_monitor_get_fd(monitor);
            if fd < 0 {
                eprintln!("Failed to get file descriptor for udev monitor");
                udev::udev_monitor_unref(monitor);
                udev::udev_unref(udev);
                return;
            }

            let mut fds = [nix::libc::pollfd {
                fd,
                events: nix::libc::POLLIN,
                revents: 0,
            }];

            println!("Listening for USB changes...");

            loop {
                let ret = nix::libc::poll(fds.as_mut_ptr(), 1, -1);
                if ret < 0 {
                    eprintln!("Poll error");
                    break;
                }

                if fds[0].revents & nix::libc::POLLIN != 0 {
                    let device = udev::udev_monitor_receive_device(monitor);
                    if !device.is_null() {
                        let action_ptr = udev::udev_device_get_action(device);
                        let devnode_ptr = udev::udev_device_get_devnode(device);
                        if !action_ptr.is_null() && !devnode_ptr.is_null() {
                            let action = CStr::from_ptr(action_ptr).to_str().unwrap();
                            let devnode = CStr::from_ptr(devnode_ptr).to_str().unwrap();

                            println!("Action: {action}, Device Node: {devnode}");

                            if action == "bind" {
                                return;
                            }
                        } else {
                            eprintln!("Received a null pointer for action or devnode");
                        }

                        udev::udev_device_unref(device);
                    } else {
                        eprintln!("Received a null device");
                    }
                }
            }

            udev::udev_monitor_unref(monitor);
            udev::udev_unref(udev);
        }
    }
}

/// Finds and, if successful, connects to the Arduino.
fn port_listener(tx: &Sender<String>) {
    // I use serialport to get vid and pid info from the serials.
    // I use serial2 for basically everything else.

    // Find port
    let mut name = String::new();
    match serialport::available_ports() {
        Ok(ports) => {
            for lport in ports {
                match lport.port_type {
                    SerialPortType::UsbPort(port) => {
                        if port.vid == 0x2341 && port.pid == 0x0042 {
                            name = lport.port_name;
                        }
                    }
                    _ => continue,
                }
            }
        }
        Err(err1) => {
            tx.send(format!("err: {err1}")).unwrap();
        }
    }
    if name.is_empty() {
        return;
    }

    // Connect and loop incoming messages
    unsafe {
        PORT = SerialPort::open(name, 9600).unwrap();
        PORT.set_read_timeout(Duration::MAX).unwrap();
        PORT.set_write_timeout(Duration::MAX).unwrap();
        PORT.set_dtr(true).unwrap();

        let mut sb = String::new();

        loop {
            let mut buf = [0; 1];
            match PORT.read(&mut buf) {
                Ok(_) => {
                    sb += &String::from_utf8(buf.to_vec()).unwrap().replace("\0", "");
                }
                Err(err2) => {
                    tx.send(format!("err: {err2}")).unwrap();
                    break;
                }
            }

            if sb.ends_with("\r\n") {
                sb = sb.replace("\r\n", "");

                if sb == "Ready" {
                    tx.send(String::from("connected")).unwrap();
                } else if sb == "testing..." {
                    tx.send(String::from("test start")).unwrap();
                } else if sb == "done" {
                    tx.send(String::from("test end")).unwrap();
                }

                sb = String::new();
            }
        }

        PORT = zeroed();
    }
}

/// Either starts listening for local communication or ends it.
#[command]
fn local_server(on: bool) {
    unsafe {
        if on {
            SRV = TcpListener::bind("192.168.1.93:8080").unwrap();
            println!("Listening on 192.168.1.93:8080");
    
            #[cfg(windows)]
            println!("Possible Windows Firewall rule addition needed");
    
            for stream in SRV.incoming() {
                let stream = stream.unwrap();
                thread::spawn(|| {
                    let addr = stream.peer_addr().unwrap().ip();
                    let mut websocket = tungstenite::accept(stream).unwrap();
                    println!("{addr} is Connected");
    
                    loop {
                        let msg = websocket.read().unwrap();
                        println!("{addr}: {msg}");
    
                        if msg.is_text() || msg.is_binary() {
                            websocket.write(msg).unwrap();
                            websocket.write("Test".into()).unwrap();
                        }
                    }
                });
            }
        } else {
            SRV = zeroed();
        }
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .setup(|app| {
            let window = app.get_webview_window("main").unwrap();
            let (tx, rx) = mpsc::channel();

            thread::spawn(move || loop {
                port_listener(&tx);
                listen_usb();
            });

            thread::spawn(move || {
                while let Ok(data) = rx.recv() {
                    if data.starts_with("err: ") {
                        window.emit("err-received", data).unwrap();
                    } else {
                        window.emit("data-received", data).unwrap();
                    }
                }
            });

            Ok(())
        })
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![send])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
