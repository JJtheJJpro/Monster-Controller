#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

#[cfg(unix)]
extern crate libudev_sys as udev;

use dialog::DialogBox;
use serialport::SerialPortType;
use std::{
    io::{stdout, Write},
    sync::mpsc::{self, Sender},
    thread::{self, spawn},
    time::Duration,
};
use tauri::{command, generate_context, generate_handler, Builder, Manager};

#[cfg(unix)]
use nix::{
    fcntl::{FcntlArg::F_SETFL, OFlag},
    ioctl_read, ioctl_write_ptr,
    libc::{cfmakeraw, tcgetattr, tcsetattr},
    poll::{self, PollFd, PollFlags},
    sys::signal::SigSet,
    unistd,
};
#[cfg(unix)]
use std::{ffi::CStr, mem::MaybeUninit, os::fd::BorrowedFd, slice};

#[cfg(windows)]
use std::{
    ffi::OsStr,
    mem::{size_of, zeroed},
    ptr::null_mut,
};
#[cfg(windows)]
use winapi::{
    shared::winerror::ERROR_IO_PENDING,
    um::{
        commapi::{EscapeCommFunction, GetCommTimeouts, SetCommState, SetCommTimeouts},
        errhandlingapi::GetLastError,
        fileapi::{CreateFileA, ReadFile, WriteFile, OPEN_EXISTING},
        handleapi::INVALID_HANDLE_VALUE,
        ioapiset::GetOverlappedResult,
        minwinbase::OVERLAPPED,
        winbase::{CBR_9600, COMMTIMEOUTS, DCB, FILE_FLAG_OVERLAPPED, SETDTR},
        winnt::{GENERIC_READ, GENERIC_WRITE},
    },
};

#[cfg(windows)]
fn check_err(m: &str) {
    unsafe {
        let err = GetLastError();
        if err > 0 {
            panic!("{m} err 0x{err:x}");
        }
    }
}

#[cfg(windows)]
fn listen_usb() {
    let com_con = wmi::COMLibrary::new().unwrap();
    let wmi_con = wmi::WMIConnection::new(com_con).unwrap();
    let _events = wmi_con
        .raw_notification::<std::collections::HashMap<String, wmi::Variant>>(
            "SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2",
        )
        .unwrap();
    //for revent in events {
    //    match revent {
    //        Ok(event) => {
    //            for (key, value) in event.iter() {
    //                println!(
    //                    "event key: '{key}' - event value: '{}'",
    //                    match value {
    //                        wmi::Variant::Empty => "Empty".to_string(),
    //                        wmi::Variant::Null => "Null".to_string(),
    //                        wmi::Variant::String(s) => s.to_string(),
    //                        wmi::Variant::I1(i1) => format!("{i1}"),
    //                        wmi::Variant::I2(i2) => format!("{i2}"),
    //                        wmi::Variant::I4(i4) => format!("{i4}"),
    //                        wmi::Variant::I8(i8) => format!("{i8}"),
    //                        wmi::Variant::R4(r4) => format!("{r4}"),
    //                        wmi::Variant::R8(r8) => format!("{r8}"),
    //                        wmi::Variant::Bool(b) => format!("{b}"),
    //                        wmi::Variant::UI1(u1) => format!("{u1}"),
    //                        wmi::Variant::UI2(u2) => format!("{u2}"),
    //                        wmi::Variant::UI4(u4) => format!("{u4}"),
    //                        wmi::Variant::UI8(u8) => format!("{u8}"),
    //                        wmi::Variant::Array(arr) => format!("{arr:?}"),
    //                        wmi::Variant::Unknown(unknown) => format!("{unknown:?}"),
    //                        wmi::Variant::Object(o) => format!("{o:?}"),
    //                    }
    //                );
    //            }
    //        }
    //        Err(e) => {
    //            eprintln!("error event: {e}");
    //        }
    //    }
    //}
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

#[cfg(windows)]
static mut PORT: u32 = 0;

#[cfg(unix)]
static mut PORT: i32 = 0;

#[command]
fn console(msg: &str) {
    println!("From Front-End: {msg}");
}

#[cfg(windows)]
#[command]
fn send(msg: &str) {
    //println!("send start");
    let r = unsafe {
        let mut overlapped: OVERLAPPED = zeroed();
        let lpmsg = OsStr::new(format!("{msg}\n").as_str())
            .as_encoded_bytes()
            .to_vec();
        let mut nbw = 0;
        let u_serial = PORT as *mut winapi::ctypes::c_void;

        WriteFile(
            u_serial,
            lpmsg.as_ptr() as *const winapi::ctypes::c_void,
            msg.len() as u32 + 1,
            &mut nbw,
            &mut overlapped,
        )
    };

    if r != 0 {
        let mut backend = dialog::backends::KDialog::new();
        backend.set_icon("error");
        dialog::Message::new(format!("Writing to arduino failed: {r}"))
            .title("Write Error")
            .show_with(backend)
            .unwrap();
        //check_err("WriteFile");
        //panic!("WriteFile spec err {r}");
    }
    //println!("send end");
}

#[cfg(unix)]
#[command]
fn send(msg: &str) {
    let fd = unsafe { BorrowedFd::borrow_raw(PORT) };

    let mut fdp = PollFd::new(fd, PollFlags::POLLOUT);

    #[cfg(target_os = "linux")]
    let wait = poll::ppoll(slice::from_mut(&mut fdp), None, Some(SigSet::empty())).unwrap();

    #[cfg(not(target_os = "linux"))]
    let wait = poll::poll(slice::from_mut(&mut fdp), 0).unwrap();

    if wait != 1 {
        eprintln!("wait != 1 (wait = {wait})");
        return;
    }

    match fdp.revents() {
        Some(e) if e == PollFlags::POLLOUT => {
            let s = unistd::write(fd, format!("{msg}\n").as_bytes()).unwrap();
            println!("sent {msg} (size {s})");
        }
        Some(e) => {
            let mut backend = dialog::backends::KDialog::new();
            backend.set_icon("error");
            dialog::Message::new(format!("Writing to arduino failed: {e:?}"))
                .title("Write Error")
                .show_with(backend)
                .unwrap();
        }
        None => {
            let mut backend = dialog::backends::KDialog::new();
            backend.set_icon("error");
            dialog::Message::new(format!("Writing to arduino failed: no event"))
                .title("Write Error")
                .show_with(backend)
                .unwrap();
        }
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
                                tx.send(String::from("connecting")).unwrap();
                                //println!("{}", lport.port_name);

                                #[cfg(windows)]
                                {
                                    let w_pn = lport.port_name + "\0";

                                    let lpname = OsStr::new(&w_pn)
                                        .as_encoded_bytes()
                                        .iter()
                                        .map(|i| *i as i8)
                                        .collect::<Vec<i8>>();

                                    let serial = unsafe {
                                        CreateFileA(
                                            lpname.as_ptr(),
                                            GENERIC_READ | GENERIC_WRITE,
                                            0,
                                            null_mut(),
                                            OPEN_EXISTING,
                                            FILE_FLAG_OVERLAPPED,
                                            null_mut(),
                                        )
                                    };

                                    //println!("{serial:?}");

                                    let err = unsafe { GetLastError() };
                                    if serial == INVALID_HANDLE_VALUE {
                                        eprintln!("CreateFileA err 0x{err:x}");
                                        break;
                                    }

                                    let mut sp: DCB = unsafe { zeroed() };
                                    sp.DCBlength = size_of::<DCB>() as u32;
                                    unsafe {
                                        SetCommState(serial, &mut sp);
                                        check_err("SetCommState");
                                    }

                                    if sp.BaudRate != CBR_9600
                                        || sp.ByteSize != 8
                                        || sp.fRtsControl() != 1
                                    {
                                        sp.BaudRate = CBR_9600;
                                        sp.ByteSize = 8;
                                        sp.set_fRtsControl(1);
                                        unsafe {
                                            SetCommState(serial, &mut sp);
                                            check_err("SetCommState");
                                        }
                                        std::thread::sleep(Duration::from_millis(100));
                                    }

                                    let mut timeout: COMMTIMEOUTS = unsafe { zeroed() };
                                    unsafe {
                                        GetCommTimeouts(serial, &mut timeout);
                                        check_err("GetCommTimeouts");
                                    }

                                    if timeout.ReadIntervalTimeout != 1
                                        || timeout.ReadTotalTimeoutConstant != 0
                                        || timeout.ReadTotalTimeoutMultiplier != 0
                                    {
                                        timeout.ReadIntervalTimeout = 1;
                                        timeout.ReadTotalTimeoutConstant = 0;
                                        timeout.ReadTotalTimeoutMultiplier = 0;
                                        unsafe {
                                            SetCommTimeouts(serial, &mut timeout);
                                            check_err("SetCommTimeouts");
                                        }
                                    }

                                    unsafe {
                                        EscapeCommFunction(serial, SETDTR);
                                    }

                                    unsafe {
                                        PORT = serial as u32;
                                    }

                                    send("On");

                                    let mut string_builder = String::new();

                                    loop {
                                        let mut buf = [0u8; 0x1F];
                                        let mut br = 0;

                                        let mut overlapped: OVERLAPPED = unsafe { zeroed() };

                                        let r = unsafe {
                                            ReadFile(
                                                serial,
                                                buf.as_mut_ptr() as *mut winapi::ctypes::c_void,
                                                0x1F,
                                                &mut br,
                                                &mut overlapped,
                                            )
                                        };

                                        if r == 1 {
                                            if br > 0 {
                                                print!(
                                                    "{}",
                                                    String::from_utf8(buf.to_vec())
                                                        .unwrap()
                                                        .replace("\0", "")
                                                );
                                                string_builder += String::from_utf8(buf.to_vec())
                                                    .unwrap()
                                                    .replace("\0", "")
                                                    .as_str();
                                                stdout().flush().unwrap();
                                            }
                                        } else if unsafe {
                                            GetLastError() == ERROR_IO_PENDING
                                                && GetOverlappedResult(
                                                    serial,
                                                    &mut overlapped,
                                                    &mut br,
                                                    1,
                                                ) == 1
                                        } {
                                            if buf[0] == 0 {
                                                continue;
                                            }
                                            print!(
                                                "{}",
                                                String::from_utf8(buf.to_vec())
                                                    .unwrap()
                                                    .replace("\0", "")
                                            );
                                            string_builder += String::from_utf8(buf.to_vec())
                                                .unwrap()
                                                .replace("\0", "")
                                                .as_str();
                                            stdout().flush().unwrap();
                                        } else {
                                            eprintln!("arduino disconnected");
                                            break;
                                        }

                                        if string_builder.ends_with("\r\n") {
                                            match string_builder.replace("\r\n", "").as_str() {
                                                "Ready" => {
                                                    tx.send(String::from("connected")).unwrap();
                                                }
                                                "testing...done" => {
                                                    tx.send(String::from("test end")).unwrap();
                                                }
                                                &_ => {}
                                            }
                                            string_builder = String::new();
                                        } else if string_builder.as_str() == "testing..." {
                                            tx.send(String::from("test start")).unwrap();
                                        }
                                    }
                                }

                                #[cfg(unix)]
                                {
                                    let w_pn = lport.port_name;

                                    let fd = nix::fcntl::open(
                                        w_pn.as_str(),
                                        OFlag::O_RDWR
                                            | OFlag::O_NOCTTY
                                            | OFlag::O_NONBLOCK
                                            | OFlag::O_CLOEXEC,
                                        nix::sys::stat::Mode::empty(),
                                    )
                                    .unwrap();

                                    nix::ioctl_none_bad!(tiocexcl, nix::libc::TIOCEXCL);

                                    unsafe {
                                        tiocexcl(fd).unwrap();
                                    }

                                    let mut termios = MaybeUninit::uninit();
                                    nix::errno::Errno::result(unsafe {
                                        tcgetattr(fd, termios.as_mut_ptr())
                                    })
                                    .unwrap();
                                    let mut termios = unsafe { termios.assume_init() };

                                    termios.c_cflag |= nix::libc::CREAD | nix::libc::CLOCAL;

                                    unsafe {
                                        cfmakeraw(&mut termios);
                                        tcsetattr(fd, nix::libc::TCSANOW, &termios);
                                    }

                                    let mut actual_termios = MaybeUninit::uninit();
                                    unsafe {
                                        tcgetattr(fd, actual_termios.as_mut_ptr());
                                    }
                                    let actual_termios = unsafe { actual_termios.assume_init() };

                                    if actual_termios.c_iflag != termios.c_iflag
                                        || actual_termios.c_oflag != termios.c_oflag
                                        || actual_termios.c_lflag != termios.c_lflag
                                        || actual_termios.c_cflag != termios.c_cflag
                                    {
                                        eprintln!("Settings did not apply correctly");
                                        continue;
                                    };

                                    #[cfg(any(target_os = "ios", target_os = "macos"))]
                                    unsafe {
                                        nix::libc::tcflush(fd, nix::libc::TCIOFLUSH);
                                    }

                                    nix::fcntl::fcntl(fd, F_SETFL(nix::fcntl::OFlag::empty()))
                                        .unwrap();

                                    let mut termios;

                                    #[cfg(any(target_os = "ios", target_os = "macos"))]
                                    {
                                        let mut t_termios = MaybeUninit::uninit();
                                        let res = unsafe {
                                            nix::libc::tcgetattr(fd, t_termios.as_mut_ptr())
                                        };
                                        nix::errno::Errno::result(res).unwrap();
                                        termios = unsafe { t_termios.assume_init() };
                                        termios.c_ispeed = 9600;
                                        termios.c_ospeed = 9600;
                                    }
                                    #[cfg(any(
                                        target_os = "dragonfly",
                                        target_os = "freebsd",
                                        target_os = "netbsd",
                                        target_os = "openbsd",
                                        all(
                                            target_os = "linux",
                                            any(
                                                target_env = "musl",
                                                target_arch = "powerpc",
                                                target_arch = "powerpc64"
                                            )
                                        )
                                    ))]
                                    {
                                        let mut t_termios = MaybeUninit::uninit();
                                        let res = unsafe {
                                            nix::libc::tcgetattr(fd, t_termios.as_mut_ptr())
                                        };
                                        nix::errno::Errno::result(res).unwrap();
                                        termios = unsafe { t_termios.assume_init() };
                                    }
                                    #[cfg(any(
                                        target_os = "android",
                                        all(
                                            target_os = "linux",
                                            not(any(
                                                target_env = "musl",
                                                target_arch = "powerpc",
                                                target_arch = "powerpc64"
                                            ))
                                        )
                                    ))]
                                    {
                                        ioctl_read!(tcgets2, b'T', 0x2A, nix::libc::termios2);

                                        let mut options = MaybeUninit::uninit();
                                        unsafe {
                                            tcgets2(fd, options.as_mut_ptr()).unwrap();
                                            termios = options.assume_init();
                                        }
                                    }

                                    // parity
                                    termios.c_cflag &= !(nix::libc::PARENB | nix::libc::PARODD);
                                    termios.c_iflag &= !nix::libc::INPCK;
                                    termios.c_iflag |= nix::libc::IGNPAR;

                                    // flow control
                                    termios.c_iflag &= !(nix::libc::IXON | nix::libc::IXOFF);
                                    termios.c_cflag &= !nix::libc::CRTSCTS;

                                    // data bits
                                    termios.c_cflag &= !nix::libc::CSIZE;
                                    termios.c_cflag |= nix::libc::CS8;

                                    // stop bits
                                    termios.c_cflag &= !nix::libc::CSTOPB;

                                    #[cfg(any(
                                        target_os = "android",
                                        all(
                                            target_os = "linux",
                                            not(any(
                                                target_env = "musl",
                                                target_arch = "powerpc",
                                                target_arch = "powerpc64"
                                            ))
                                        )
                                    ))]
                                    {
                                        termios.c_cflag &= !nix::libc::CBAUD;
                                        termios.c_cflag |= nix::libc::BOTHER;
                                        termios.c_ispeed = 9600;
                                        termios.c_ospeed = 9600;
                                    }
                                    #[cfg(any(
                                        target_os = "dragonfly",
                                        target_os = "freebsd",
                                        target_os = "netbsd",
                                        target_os = "openbsd"
                                    ))]
                                    unsafe {
                                        nix::libc::cfsetspeed(termios, 9600);
                                    }
                                    #[cfg(all(
                                        target_os = "linux",
                                        any(
                                            target_env = "musl",
                                            target_arch = "powerpc",
                                            target_arch = "powerpc64"
                                        )
                                    ))]
                                    unsafe {
                                        let res = nix::libc::cfsetspeed(termios, 9600);
                                        nix::errno::Errno::result(res).unwrap();
                                    }

                                    #[cfg(any(target_os = "ios", target_os = "macos"))]
                                    {
                                        nix::ioctl_write_ptr_bad!(
                                            iossiospeed,
                                            0x80045402,
                                            nix::libc::speed_t
                                        );

                                        let res = unsafe {
                                            nix::libc::tcsetattr(fd, nix::libc::TCSANOW, termios)
                                        };
                                        nix::errno::Errno::result(res).unwrap();
                                        iossiospeed(bd, &9600);
                                    }
                                    #[cfg(any(
                                        target_os = "dragonfly",
                                        target_os = "freebsd",
                                        target_os = "netbsd",
                                        target_os = "openbsd",
                                        all(
                                            target_os = "linux",
                                            any(
                                                target_env = "musl",
                                                target_arch = "powerpc",
                                                target_arch = "powerpc64"
                                            )
                                        )
                                    ))]
                                    {
                                        let res = unsafe {
                                            nix::libc::tcsetattr(fd, nix::libc::TCSANOW, termios)
                                        };
                                        nix::errno::Errno::result(res).unwrap();
                                    }
                                    #[cfg(any(
                                        target_os = "android",
                                        all(
                                            target_os = "linux",
                                            not(any(
                                                target_env = "musl",
                                                target_arch = "powerpc",
                                                target_arch = "powerpc64"
                                            ))
                                        )
                                    ))]
                                    unsafe {
                                        ioctl_write_ptr!(tcsets2, b'T', 0x2B, nix::libc::termios2);
                                        tcsets2(fd, &termios).unwrap();
                                    }

                                    unsafe {
                                        PORT = fd;
                                    }

                                    let mut string_builder = String::new();

                                    loop {
                                        let mut fdp = PollFd::new(
                                            unsafe { BorrowedFd::borrow_raw(fd) },
                                            PollFlags::POLLIN,
                                        );

                                        #[cfg(target_os = "linux")]
                                        let wait = nix::poll::ppoll(
                                            std::slice::from_mut(&mut fdp),
                                            None,
                                            Some(SigSet::empty()),
                                        )
                                        .unwrap();

                                        #[cfg(not(target_os = "linux"))]
                                        let wait =
                                            nix::poll::poll(std::slice::from_mut(&mut fdp), 0)
                                                .unwrap();

                                        if wait != 1 {
                                            eprintln!("wait != 1 (wait = {wait})");
                                            break;
                                        }

                                        match fdp.revents() {
                                            Some(e) if e == PollFlags::POLLIN => {
                                                let mut buf = [0; 1];
                                                nix::unistd::read(fd, &mut buf).unwrap();
                                                //print!("{}", buf[0] as char);
                                                string_builder +=
                                                    format!("{}", buf[0] as char).as_str();
                                                stdout().flush().unwrap();
                                            }
                                            Some(_) | None => panic!("read error"),
                                        }

                                        if string_builder.ends_with("\r\n") {
                                            match string_builder.replace("\r\n", "").as_str() {
                                                "Ready" => {
                                                    tx.send(String::from("connected")).unwrap();
                                                }
                                                "testing...done" => {
                                                    tx.send(String::from("test end")).unwrap();
                                                }
                                                &_ => {
                                                    //print!("string_builder ({}) = {}", string_builder.len(), string_builder.replace("\r", "\\r").replace("\n", "\\n").replace("\0", "\\0"));
                                                    //stdout().flush().unwrap();
                                                }
                                            }
                                            string_builder = String::new();
                                        } else if string_builder.as_str() == "testing.." {
                                            tx.send(String::from("test start")).unwrap();
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

fn main() {
    Builder::default()
        .setup(|app| {
            let window = app.get_window("main").unwrap();
            let (tx, rx) = mpsc::channel();

            port_listener(tx);

            spawn(move || {
                thread::sleep(Duration::from_millis(500));
                while let Ok(data) = rx.recv() {
                    //println!("{data}");
                    window.emit("data-received", data).unwrap();
                }
            });

            Ok(())
        })
        .invoke_handler(generate_handler![send, console])
        //.invoke_handler(generate_handler![console])
        .run(generate_context!())
        .expect("error while running tauri application");
}
