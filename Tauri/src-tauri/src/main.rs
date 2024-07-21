#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

//#[cfg(unix)]
//extern crate libudev_sys as udev;

//use dialog::DialogBox;
use serialport::SerialPortType;
use std::{
    mem::zeroed,
    net::TcpListener,
    sync::mpsc::{self, Sender},
    thread::{self, spawn},
    time::Duration,
};
use tauri::{command, generate_context, generate_handler, Builder, Manager};

//#[cfg(unix)]
//use nix::{
//    fcntl::{FcntlArg::F_SETFL, OFlag},
//    ioctl_read, ioctl_write_ptr,
//    libc::{cfmakeraw, tcgetattr, tcsetattr},
//    poll::{self, PollFd, PollFlags},
//    sys::signal::SigSet,
//    unistd,
//};
//#[cfg(unix)]
//use std::{ffi::CStr, mem::MaybeUninit, os::fd::BorrowedFd, slice};

#[cfg(windows)]
fn listen_usb() {
    let com_con = wmi::COMLibrary::new().unwrap();
    let wmi_con = wmi::WMIConnection::new(com_con).unwrap();
    let _events = wmi_con
        .raw_notification::<std::collections::HashMap<String, wmi::Variant>>(
            "SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2",
        )
        .unwrap();
}

#[cfg(unix)]
fn listen_usb() {
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

static mut PORT: serial2::SerialPort = unsafe { zeroed() };

#[command]
fn send(msg: &str) {
    let m = format!("{msg}\n");
    unsafe {
        PORT.write(m.as_bytes()).unwrap();
    }
}

fn port_listener(tx: Sender<String>) {
    spawn(move || loop {
        //let mut t_pname_c: Vec<String> = Vec::new();
        match serialport::available_ports() {
            Ok(ports) => {
                //let temp: Vec<String> = ports.clone().into_iter().map(|s| s.port_name).collect();
                //if t_pname_c == temp {}
                for lport in ports {
                    match lport.port_type {
                        SerialPortType::UsbPort(port) => {
                            if port.vid == 0x2341 && port.pid == 0x0042 {
                                let mut sport =
                                    serial2::SerialPort::open(lport.port_name, 9600).unwrap();
                                sport.set_read_timeout(Duration::MAX).unwrap();
                                sport.set_write_timeout(Duration::MAX).unwrap();
                                sport.set_dtr(true).unwrap();
                                unsafe {
                                    PORT = sport.try_clone().unwrap();
                                }

                                let mut string_builder = String::new();

                                loop {
                                    let mut buf = [0; 1];
                                    match sport.read(&mut buf) {
                                        Ok(_) => {
                                            print!(
                                                "{}",
                                                String::from_utf8(buf.to_vec())
                                                    .unwrap()
                                                    .replace("\0", "")
                                            );
                                            string_builder += &String::from_utf8(buf.to_vec())
                                                .unwrap()
                                                .replace("\0", "");
                                        }
                                        Err(e) => {
                                            eprintln!("{e}");
                                            break;
                                        }
                                    }

                                    if string_builder.ends_with("\r\n") {
                                        string_builder = string_builder.replace("\r\n", "");

                                        if string_builder == "Ready" {
                                            tx.send(String::from("connected")).unwrap();
                                        } else if string_builder == "testing..." {
                                            tx.send(String::from("test start")).unwrap();
                                        } else if string_builder == "done" {
                                            tx.send(String::from("test end")).unwrap();
                                        }
                                    }
                                }
                            }
                        }
                        _ => continue,
                    }
                }
            }
            Err(err) => {
                println!("{}", err);
                tx.send(String::from("available_ports err")).unwrap();
            }
        }

        //tx.send(String::from("listen usb")).unwrap();

        // before another loop, listen for usb changes
        listen_usb();
    });
}

static mut SRV: TcpListener = unsafe { zeroed() };

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
                spawn(|| {
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

fn main() {
    Builder::default()
        .setup(|app| {
            let window = app.get_window("main").unwrap();
            let (tx, rx) = mpsc::channel();

            port_listener(tx);

            spawn(|| local_server(true));

            spawn(move || {
                thread::sleep(Duration::from_millis(500));
                while let Ok(data) = rx.recv() {
                    //println!("{data}");
                    window.emit("data-received", data).unwrap();
                }
            });

            Ok(())
        })
        .invoke_handler(generate_handler![send, local_server])
        //.invoke_handler(generate_handler![console])
        .run(generate_context!())
        .expect("error while running tauri application");
}
