namespace MonsterController
{
    public partial class MainForm : Form
    {
        private bool[] data =
        [
            false, false, false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false, false, false,
            false, false, false, false, false, false, false, false, false, false,
            false, false
        ];

        public MainForm()
        {
            InitializeComponent();
        }

        private void M1Name_Click(object sender, EventArgs e)
        {
            PortCom.SendData($"{((data[0] = !data[0]) ? 1 : 0)}01");
        }
    }
}
