using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace MonsterController.ViewModels;

public class MainWindowViewModel : ViewModelBase
{
    private List<string> _monsterNames = [
        "Monster 1",
        "Monster 2",
        "Monster 3",
        "Monster 4",
        "Monster 5",
        "Monster 6",
        "Monster 7",
        "Monster 8",
        "Monster 9",
        "Monster 10",
    ];

    public string Monster1
    {
        get => _monsterNames[0];
        set
        {
            if (_monsterNames[0] != value)
            {
                _monsterNames[0] = value;
                OnPropertyChanged(nameof(Monster1));
            }
        }
    }
    public string Monster2
    {
        get => _monsterNames[1];
        set
        {
            if (_monsterNames[1] != value)
            {
                _monsterNames[1] = value;
                OnPropertyChanged();
            }
        }
    }
    public string Monster3
    {
        get => _monsterNames[2];
        set
        {
            if (_monsterNames[2] != value)
            {
                _monsterNames[2] = value;
                OnPropertyChanged();
            }
        }
    }
    public string Monster4
    {
        get => _monsterNames[3];
        set
        {
            if (_monsterNames[3] != value)
            {
                _monsterNames[3] = value;
                OnPropertyChanged();
            }
        }
    }
    public string Monster5
    {
        get => _monsterNames[4];
        set
        {
            if (_monsterNames[4] != value)
            {
                _monsterNames[4] = value;
                OnPropertyChanged();
            }
        }
    }
    public string Monster6
    {
        get => _monsterNames[5];
        set
        {
            if (_monsterNames[5] != value)
            {
                _monsterNames[5] = value;
                OnPropertyChanged();
            }
        }
    }
    public string Monster7
    {
        get => _monsterNames[6];
        set
        {
            if (_monsterNames[6] != value)
            {
                _monsterNames[6] = value;
                OnPropertyChanged();
            }
        }
    }
    public string Monster8
    {
        get => _monsterNames[7];
        set
        {
            if (_monsterNames[7] != value)
            {
                _monsterNames[7] = value;
                OnPropertyChanged();
            }
        }
    }
    public string Monster9
    {
        get => _monsterNames[8];
        set
        {
            if (_monsterNames[8] != value)
            {
                _monsterNames[8] = value;
                OnPropertyChanged();
            }
        }
    }
    public string Monster10
    {
        get => _monsterNames[9];
        set
        {
            if (_monsterNames[9] != value)
            {
                _monsterNames[9] = value;
                OnPropertyChanged(nameof(Monster10));
            }
        }
    }

    public new event PropertyChangedEventHandler? PropertyChanged;
    public void OnPropertyChanged([CallerMemberName] string? propertyName = null)
    {
        PropertyChanged?.Invoke(this, new(propertyName));
    }
}
