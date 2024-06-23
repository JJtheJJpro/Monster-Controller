using System;
using System.IO.Ports;
using System.Runtime.InteropServices;
using System.Runtime.Versioning;

#if WINDOWS
using System.Management;
#endif

namespace MonsterController;

internal static partial class PortCom
{
#if LINUX
    private const string LibUdev = "libudev.so.1";
    private const string LibC = "libc";

    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial nint udev_new();

    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial void udev_unref(nint udev);

    [LibraryImport(LibUdev, StringMarshalling = StringMarshalling.Custom, StringMarshallingCustomType = typeof(System.Runtime.InteropServices.Marshalling.AnsiStringMarshaller))]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial nint udev_monitor_new_from_netlink(nint udev, [MarshalAs(UnmanagedType.LPStr)] string name);

    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial int udev_monitor_enable_receiving(nint monitor);

    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial nint udev_monitor_get_fd(nint monitor);

    [LibraryImport(LibUdev)]
    [UnmanagedCallConv(CallConvs = [typeof(System.Runtime.CompilerServices.CallConvCdecl)])]
    private static partial nint udev_monitor_receive_device(nint monitor);

    [DllImport(LibC)]
    private static extern int poll(nint fds, uint nfds, int timemout);
#elif MACOS
    private const string IOKitFramework = "/System/Library/Frameworks/IOKit.framework/IOKit";

    [LibraryImport(IOKitFramework)]
    private static partial nint IOServiceAddMatchingNotification(
        nint notifyPort,
        [MarshalAs(UnmanagedType.LPStr)] string notificationType,
        nint matching,
        IOServiceMatchingCallback callback,
        nint refCon,
        out IntPtr notification);

    [LibraryImport(IOKitFramework)]
    private static partial void IOObjectRelease(nint obj);

    private delegate void IOServiceMatchingCallback(IntPtr refCon, IntPtr service, uint messageType, IntPtr messageArgument);
#endif

    public static void FindPort()
    {
#pragma warning disable CA1416
        ListenForUSBChanges();
#pragma warning restore CA1416
    }

#if WINDOWS
    [SupportedOSPlatform("windows")]
    public static void ListenForUSBChanges()
    {
        WqlEventQuery query = new("SELECT * FROM Win32_DeviceChangeEvent WHERE EventType = 2");
        ManagementEventWatcher watcher = new(query);
        watcher.WaitForNextEvent();
    }
#elif LINUX
    [SupportedOSPlatform("linux")]
    public static void ListenForUSBChanges()
    {
        nint udev = udev_new();
        nint monitor = udev_monitor_new_from_netlink(udev, "udev");
        udev_monitor_enable_receiving(monitor);

        int fd = Convert.ToInt32(udev_monitor_get_fd(monitor));

        nint[] fds = new nint[1];
        fds[0] = (nint)fd;

        while (true)
        {
            int ret = poll(fds, 1, -1);
        }

        udev_unref(udev);
    }
#elif MACOS
    [SupportedOSPlatform("osx")]
    public static void ListenForUSBChanges()
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
                DeviceNotificationCallback,
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
#endif
}