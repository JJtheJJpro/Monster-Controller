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

//#[cfg(unix)]
//use std::os::fd::{AsRawFd, RawFd};

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
#[cfg(windows)]
fn listen_usb() {
    #[cfg(windows)]
    {
        let com_con = wmi::COMLibrary::new().unwrap();
        let wmi_con = wmi::WMIConnection::new(com_con).unwrap();

        for _ in wmi_con
            .raw_notification::<std::collections::HashMap<String, wmi::Variant>>(
                "SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2",
            )
            .unwrap()
        {
            break;
        }
    }
    /*
    #[cfg(unix)]
    {
        let context = libudev::Context::new().unwrap();
        let mut monitor = libudev::Monitor::new(&context).unwrap();
        monitor.match_subsystem("usb").unwrap();
        let mut monitor_socket = monitor.listen().unwrap();

        let fd: RawFd = monitor_socket.as_raw_fd();

        let mut fds = [libc::pollfd {
            fd,
            events: libc::POLLIN,
            revents: 0,
        }];

        loop {
            let res = unsafe { libc::poll(fds.as_mut_ptr(), 1, -1) };
            if res == -1 {
                eprintln!("Poll failed: {}", std::io::Error::last_os_error());
                break;
            }

            if fds[0].revents & libc::POLLIN != 0 {
                match monitor_socket.receive_event() {
                    Some(event) => match event.event_type() {
                        libudev::EventType::Add => {
                            break;
                        }
                        _ => continue,
                    },
                    None => eprintln!("Failed to receive event"),
                }
            }
        }
    }
    */
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

    tx.send(String::from("connecting")).unwrap();

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
                    tx.send(String::from("disconnected")).unwrap();
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
    thread::spawn(move || unsafe {
        if on {
            let ip = local_ip_address::local_ip().unwrap_or(std::net::IpAddr::V4(
                std::net::Ipv4Addr::new(192, 186, 1, 1),
            ));

            SRV = TcpListener::bind(format!("{}:8080", ip)).unwrap();
            println!("Listening on {}:8080", ip);

            #[cfg(windows)]
            println!("Possible Windows Firewall rule addition needed");

            while let Ok(stream) = SRV.accept() {
                thread::spawn(|| {
                    let addr = stream.0.peer_addr().unwrap().ip();
                    let mut websocket = tungstenite::accept(stream.0).unwrap();
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
    });
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .setup(|app| {
            let window = app.get_webview_window("main").unwrap();
            let (tx, rx) = mpsc::channel();

            thread::spawn(move || loop {
                port_listener(&tx);

                #[cfg(windows)]
                listen_usb();

                #[cfg(unix)]
                thread::sleep(Duration::from_secs(10));
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
        .invoke_handler(tauri::generate_handler![send, local_server])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
