using System;
using System.IO.Ports;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;
using System.Collections.Generic;


#if WINDOWS
using System.Management;
#endif

namespace MonsterController;

internal static partial class PortCom
{
    [SupportedOSPlatform("linux")]
    private const string LibUdev = "libudev.so.1";
    [SupportedOSPlatform("linux")]
    private const string LibC = "libc";

    [SupportedOSPlatform("linux")]
    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial nint udev_new();

    [SupportedOSPlatform("linux")]
    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial void udev_unref(nint udev);

    [SupportedOSPlatform("linux")]
    [LibraryImport(LibUdev, StringMarshalling = StringMarshalling.Custom, StringMarshallingCustomType = typeof(System.Runtime.InteropServices.Marshalling.AnsiStringMarshaller))]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial nint udev_monitor_new_from_netlink(nint udev, [MarshalAs(UnmanagedType.LPStr)] string name);

    [SupportedOSPlatform("linux")]
    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial int udev_monitor_enable_receiving(nint monitor);

    [SupportedOSPlatform("linux")]
    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial nint udev_monitor_get_fd(nint monitor);

    [SupportedOSPlatform("linux")]
    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial nint udev_monitor_receive_device(nint monitor);

    [SupportedOSPlatform("linux")]
    [LibraryImport(LibC)]
    private static partial int poll(nint fds, uint nfds, int timemout);

    [SupportedOSPlatform("osx")]
    private const string IOKitFramework = "/System/Library/Frameworks/IOKit.framework/IOKit";

    [SupportedOSPlatform("osx")]
    [LibraryImport(IOKitFramework)]
    private static partial nint IOServiceAddMatchingNotification(
        nint notifyPort,
        [MarshalAs(UnmanagedType.LPStr)] string notificationType,
        nint matching,
        IOServiceMatchingCallback callback,
        nint refCon,
        out IntPtr notification);

    [SupportedOSPlatform("osx")]
    [LibraryImport(IOKitFramework)]
    private static partial void IOObjectRelease(nint obj);

    private delegate void IOServiceMatchingCallback(IntPtr refCon, IntPtr service, uint messageType, IntPtr messageArgument);

    public static void FindPort()
    {
        foreach (KeyValuePair<string, Tuple<uint, uint>> portInfo in GetIDs())
        {
            if (portInfo.Value.Item1 == 0x2341 && portInfo.Value.Item2 == 0x0042)
            {
                try
                {
                    SerialPort port = new(portInfo.Key, 9600)
                    {
                        ReadTimeout = -1,
                        WriteTimeout = -1
                    };
                    port.Open();
                    while (port.IsOpen)
                    {
                        string input = port.ReadLine();
                        Console.WriteLine($"{input}");
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e);
                }
            }
        }

        ListenForUSBChanges();
    }

    public static void ListenForUSBChanges()
    {
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            WqlEventQuery query = new("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2");
            ManagementEventWatcher watcher = new(query);
            watcher.WaitForNextEvent();
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
        {
            nint udev = udev_new();
            nint monitor = udev_monitor_new_from_netlink(udev, "udev");
            int hr = udev_monitor_enable_receiving(monitor);

            int fd = Convert.ToInt32(udev_monitor_get_fd(monitor));

            //nint[] fds = [fd];

            while (true)
            {
                int ret = poll(fd, 1, -1);
                break;
            }

            udev_unref(udev);
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
        {
            IntPtr notifyPort = IntPtr.Zero;
            IntPtr matchingDict = IntPtr.Zero;
            IntPtr notification = IntPtr.Zero;

            try
            {
                notifyPort = IntPtr.Zero; // Obtain the IONotificationPort

                matchingDict = IntPtr.Zero; // Create a matching dictionary for USB devices

                // Add notification callback for USB devices
                IOServiceAddMatchingNotification(
                    notifyPort,
                    "IOServiceMatching",
                    matchingDict,
                    new((con, service, msgType, msgArg) =>
                    {

                    }),
                    IntPtr.Zero,
                    out notification);

                // Run your event loop here to process USB device events
                // Implement your logic to handle USB device events here
            }
            finally
            {
                if (notifyPort != IntPtr.Zero)
                {
                    IOObjectRelease(notifyPort);
                }
                if (notification != IntPtr.Zero)
                {
                    IOObjectRelease(notification);
                }
            }
        }
    }

    public static Dictionary<string, Tuple<uint, uint>> GetIDs()
    {
        if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            foreach (string portName in SerialPort.GetPortNames())
            {
                using var searcher = new ManagementObjectSearcher($"SELECT * FROM Win32_PnPEntity WHERE Caption LIKE '%({portName})%'");
                foreach (ManagementBaseObject obj in searcher.Get())
                {
                    string deviceID = (string)obj["DeviceID"];
                    Console.WriteLine($"DeviceID: {deviceID}");

                    string[] idParts = deviceID.Split('\\');
                    if (idParts.Length > 1)
                    {
                        string[] vidPidParts = idParts[1].Split('&');
                        foreach (string part in vidPidParts)
                        {
                            if (part.StartsWith("VID_", StringComparison.OrdinalIgnoreCase))
                            {
                                Console.WriteLine($"VID: {part[4..]}");
                            }
                            else if (part.StartsWith("PID_", StringComparison.OrdinalIgnoreCase))
                            {
                                Console.WriteLine($"PID: {part[4..]}");
                            }
                        }
                    }
                }
            }
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
        {
            throw new NotImplementedException("linux not supported yet");
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
        {
            throw new NotImplementedException("macos not supported yet");
        }
        else if (RuntimeInformation.IsOSPlatform(OSPlatform.FreeBSD))
        {
            throw new NotImplementedException("macos not supported yet");
        }
        throw new PlatformNotSupportedException("whatever platform you're using, it will never be supported, until C# can up it's game, you know?");
    }
}