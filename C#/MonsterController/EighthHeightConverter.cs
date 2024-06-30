using Avalonia.Data.Converters;
using System;
using System.Globalization;

namespace MonsterController;

public class EighthHeightConverter : IValueConverter
{
    public object? Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is double windowHeight)
        {
            return windowHeight / 8;
        }
        return 0;
    }

    public object? ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}