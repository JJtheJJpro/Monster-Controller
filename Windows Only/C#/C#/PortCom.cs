using System.Diagnostics;
using System.IO.Ports;

namespace MonsterController
{
    internal static class PortCom
    {
        public static SerialPort sp = new("COM5", 9600)
        {
            DtrEnable = true // to reset when connected (makes it faster to connect fully)
        };

        public static void Connect()
        {
            sp.DataReceived += DataReceived;
            sp.Open();
        }
        public static void SendData(string data)
        {
            sp.WriteLine(data);
        }
        internal static void DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            string input = sp.ReadLine();
            Debug.Write(input);
        }
    }
}
