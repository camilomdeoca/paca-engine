#pragma once

#include <QVariant>

#include <string>
#include <type_traits>

namespace qt_utils {

inline QVariant toQVariant(const std::string &str)
{
    return QString::fromStdString(str);
}

inline QVariant toQVariant(std::string_view str)
{
    return QString::fromStdString(std::string(str));
}

template <typename T>
requires std::is_arithmetic_v<T>
QVariant toQVariant(T value)
{
    return value;
}

template <typename T>
requires std::is_enum_v<T>
QVariant toQVariant(T value)
{
    return std::to_underlying(value);
}

template <typename T>
requires requires(T t)
{
    { t.getClassName() } -> std::same_as<std::string_view>;
}

inline QVariant toQVariant(T value)
{
    return toQVariant(value.getClassName());
}

inline QVariant toQVariant(auto value)
{
    return typeid(value).name();
}

}
