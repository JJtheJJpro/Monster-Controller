using System;
using System.Threading;
using System.Threading.Tasks;
using Avalonia.Controls;
using Avalonia.Interactivity;
using MonsterController.ViewModels;
using MsBox.Avalonia;
using MsBox.Avalonia.Base;
using MsBox.Avalonia.Dto;

namespace MonsterController.Views;

public partial class MainWindow : Window
{
    private (int, DateTime) _lastClickTime = (0, DateTime.MinValue);

    private async Task<string?> GetMonsterNameInput()
    {
        IMsBox<string> msgBox = MessageBoxManager.GetMessageBoxCustom(new MessageBoxCustomParams
        {
            ButtonDefinitions = [
                    new() { Name = "Ok" },
                    new() { Name = "Cancel" },
                ],
            InputParams = new()
            {
                DefaultValue = DataContext is MainWindowViewModel model ? model.Monster1 : "Monster 1",
                Multiline = false
            },
            WindowStartupLocation = WindowStartupLocation.CenterOwner,
            ContentTitle = "New Monster Name",
            ContentMessage = "Enter in the new monster name:"
        });

        string result = await msgBox.ShowWindowDialogAsync(this);
        if (result == "Ok")
        {
            //await MessageBoxManager.GetMessageBoxStandard("This", msgBox.InputValue).ShowWindowDialogAsync(this);
            return msgBox.InputValue;
        }
        return null;
    }

    #region Monster Names Change
    private async void MCM1(object sender, RoutedEventArgs e)
    {
        if (_lastClickTime.Item1 == 1 && (DateTime.Now - _lastClickTime.Item2).TotalMilliseconds < 300)
        {
            string? newName = await GetMonsterNameInput();
            if (newName != null && DataContext is MainWindowViewModel model)
            {
                model.Monster1 = newName;
                model.OnPropertyChanged("Monster1");
            }
            _lastClickTime = (0, DateTime.MinValue);
        }
        else
        {
            _lastClickTime = (1, DateTime.Now);
        }
    }
    #endregion

    public MainWindow()
    {
        InitializeComponent();
        DataContext = new MainWindowViewModel();
    }
}