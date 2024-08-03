using Android.Content;
using Android.Hardware.Usb;
using System.IO.Ports;
using A = Android;

namespace MonsterController.Platforms.Android
{
    internal class SPProxy
    {


		private UsbManager? _usbManager;
		private UsbDevice? _device;
		private UsbDeviceConnection? _connection;
		private UsbEndpoint? _readEndpoint;
		private UsbEndpoint? _writeEndpoint;

		// USB control request type constants
		private const int USB_TYPE_STANDARD = (0x00 << 5);
		private const int USB_TYPE_CLASS = (0x01 << 5);
		private const int USB_TYPE_VENDOR = (0x02 << 5);
		private const int USB_TYPE_RESERVED = (0x03 << 5);

		// USB recipient constants
		private const int USB_RECIP_DEVICE = 0x00;
		private const int USB_RECIP_INTERFACE = 0x01;
		private const int USB_RECIP_ENDPOINT = 0x02;
		private const int USB_RECIP_OTHER = 0x03;

		// FTDI-specific request codes
		private const int FTDI_SIO_SET_BAUDRATE_REQUEST = 0x03;
		private const int FTDI_SIO_SET_DATA_REQUEST = 0x04;

		public SPProxy()
		{
			_usbManager = (UsbManager?)A.App.Application.Context.GetSystemService(Context.UsbService);
		}

		public void OpenSerialPort(string deviceName, int baudRate)
		{
			foreach (var usbDevice in _usbManager?.DeviceList?.Values ?? throw new Exception())
			{
				if (usbDevice.DeviceName == deviceName)
				{
					_device = usbDevice;
					break;
				}
			}

			if (_device == null)
				throw new Exception("Device not found");

			_connection = _usbManager.OpenDevice(_device);
			if (_connection == null)
				throw new Exception("Could not open connection");

			UsbInterface usbInterface = _device.GetInterface(0);
			_connection.ClaimInterface(usbInterface, true);

			for (int i = 0; i < usbInterface.EndpointCount; i++)
			{
				UsbEndpoint? endpoint = usbInterface.GetEndpoint(i);
				if (endpoint?.Type == UsbAddressing.XferBulk)
				{
					if (endpoint.Direction == UsbAddressing.In)
						_readEndpoint = endpoint;
					else if (endpoint.Direction == UsbAddressing.Out)
						_writeEndpoint = endpoint;
				}
			}

			if (_readEndpoint == null || _writeEndpoint == null)
				throw new Exception("Endpoints not found");

			SetParameters(baudRate);
		}

		private void SetParameters(int baudRate, int dataBits = 8, StopBits stopBits = StopBits.One, Parity parity = Parity.None)
		{
			if (_connection == null || _device == null)
			{
				throw new InvalidOperationException("Device not connected");
			}

			// Set baud rate
			SetBaudRate(baudRate);

			// Set data bits, stop bits, and parity
			int config = dataBits;

			switch (parity)
			{
				case Parity.None:
					config |= 0x00;
					break;
				case Parity.Odd:
					config |= 0x100;
					break;
				case Parity.Even:
					config |= 0x200;
					break;
				case Parity.Mark:
					config |= 0x300;
					break;
				case Parity.Space:
					config |= 0x400;
					break;
			}

			switch (stopBits)
			{
				case StopBits.One:
					config |= 0x00;
					break;
				case StopBits.Two:
					config |= 0x1000;
					break;
				case StopBits.OnePointFive:
					config |= 0x2000;
					break;
			}

			// Send control transfer to set data characteristics
			int result = _connection.ControlTransfer(
				UsbAddressing.Out | (UsbAddressing)USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				FTDI_SIO_SET_DATA_REQUEST,
				config,
				0,
				null,
				0,
				1000
			);

			if (result < 0)
			{
				throw new IOException("Failed to set data parameters");
			}
		}

		private void SetBaudRate(int baudRate)
		{
			int divisor = 24000000 / baudRate; // Example calculation for FTDI chip

			// Send control transfer to set baud rate
			int? result = _connection?.ControlTransfer(
				UsbAddressing.Out | (UsbAddressing)USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				FTDI_SIO_SET_BAUDRATE_REQUEST,
				(divisor & 0xFFFF), // Low 16 bits
				(divisor >> 16), // High 16 bits
				null,
				0,
				1000
			);

			if (result < 0)
			{
				throw new IOException("Failed to set baud rate");
			}
		}

		public void CloseSerialPort()
		{
			_connection?.ReleaseInterface(_device?.GetInterface(0));
			_connection?.Close();
			_device = null;
			_connection = null;
			_readEndpoint = null;
			_writeEndpoint = null;
		}

		public void WriteData(byte[] data)
		{
			if (_connection == null || _writeEndpoint == null)
				throw new Exception("Serial port not open");

			_connection.BulkTransfer(_writeEndpoint, data, data.Length, 1000);
		}

		public byte[] ReadData()
		{
			if (_connection == null || _readEndpoint == null)
				throw new Exception("Serial port not open");

			byte[] buffer = new byte[1024];
			int bytesRead = _connection.BulkTransfer(_readEndpoint, buffer, buffer.Length, 1000);
			if (bytesRead < 0)
				return [];

			byte[] result = new byte[bytesRead];
			Array.Copy(buffer, result, bytesRead);
			return result;
		}
	}
}
