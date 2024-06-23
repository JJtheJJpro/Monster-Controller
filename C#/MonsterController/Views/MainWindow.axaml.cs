using Avalonia.Controls;

namespace MonsterController.Views;

using ViewModels;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        MainWindowViewModel.Greeting = "bruh";
    }
}