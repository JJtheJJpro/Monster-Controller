use std::io::Write;

use serialport::{Error, SerialPortType};

#[cfg(windows)]
fn listen_for_changes() {
    let com_con = wmi::COMLibrary::new().unwrap();
    let wmi_con = wmi::WMIConnection::new(com_con).unwrap();
    let _events = wmi_con
        .raw_notification::<std::collections::HashMap<String, wmi::Variant>>(
            "SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2",
        )
        .unwrap();
}

#[cfg(unix)]
fn listen_for_changes() {
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

fn find_port() -> Result<Option<String>, Error> {
    match serialport::available_ports() {
        Ok(ports) => {
            for lport in ports {
                match lport.port_type {
                    SerialPortType::UsbPort(port) => {
                        if port.vid == 0x2341 && port.pid == 0x0042 {
                            return Ok(Some(lport.port_name));
                        } else {
                            continue;
                        }
                    }
                    _ => {
                        continue;
                    }
                }
            }
            Ok(None)
        }
        Err(e) => Err(e),
    }
}

fn main() {
    let args: Vec<String> = std::env::args().collect();

    println!("{args:?}");

    if args[1] == String::from("-ui") {
        println!("Running seperate UI...");
        // run ui
        println!("Program exited with code 0");
        return;
    }

    loop {
        print!("");
        std::io::stdout().flush().unwrap();

        match find_port() {
            Ok(o_s) => match o_s {
                Some(_) => {}
                None => {}
            },
            Err(_) => {}
        }
        listen_for_changes();
    }
}
