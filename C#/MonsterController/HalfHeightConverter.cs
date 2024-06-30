using Avalonia.Data.Converters;
using System;
using System.Globalization;

namespace MonsterController;

public class HalfHeightConverter : IValueConverter
{
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is double windowHeight)
        {
            return windowHeight / 2;
        }
        return 0;
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}