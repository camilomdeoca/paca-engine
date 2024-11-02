#pragma once

#include <glm/glm.hpp>

#include <ostream>
#include <istream>
#include <vector>
#include <array>
#include <type_traits>

#include <boost/pfr.hpp>

#ifdef LOG_EVERY_SERIALIZATION
    #include <iostream>
    #define LOG_SERIALIZATION(variable) { std::cout << variable << std::endl; }
#else
    #define LOG_SERIALIZATION(variable)
#endif

namespace paca::fileformats {

template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref>: std::true_type {};

template <template<typename...> class Ref, typename... Args>
constexpr bool is_specialization_v = is_specialization<Ref<Args...>, Ref>::value;

template <typename T>
concept dynamic_array
    = is_specialization<T, std::vector>::value
    || is_specialization<T, std::basic_string>::value;

template <typename T>
concept glm_type = requires(T t) {
    { t.length() } -> std::same_as<int>;
    { t[0] } -> std::same_as<decltype(t[0])&>;
};

class Serializer
{
public:
    Serializer(std::ostream &os)
        : m_os(os)
    {}

    void operator()(const glm_type auto &field)
    {
        for (decltype(field.length()) i = 0; i < field.length(); i++)
            (*this)(field[i]);
    }

    void operator()(const dynamic_array auto &field)
    {
        using size_type = decltype(field.size());
        const size_type size = field.size();

        LOG_SERIALIZATION("Dynamic aggregate of size: " << size);
        m_os.write(reinterpret_cast<const char*>(&size), sizeof(size));
        for (size_type i = 0; i < size; i++)
        {
            (*this)(field[i]);
        }
    }

    template <class ElementType, typename SizeType, SizeType size>
    void operator()(const std::array<ElementType, size> &field)
    {
        LOG_SERIALIZATION("Serializing std::array of size: " << size);
        for (SizeType i = 0; i < size; i++)
        {
            (*this)(field[i]);
        }
    }

    template <typename T>
    requires std::is_arithmetic_v<T> || std::is_enum_v<T>
    void operator()(const T &field)
    {
        LOG_SERIALIZATION(field);
        m_os.write(reinterpret_cast<const char*>(&field), sizeof(field));
    }

    
    template <typename T>
    requires std::is_aggregate_v<T>
    void operator()(const T &object) {
        boost::pfr::for_each_field(object, [this](const auto &field) {
            (*this)(field);
        });
    }

private:
    std::ostream &m_os;
};

class Unserializer
{
public:
    Unserializer(std::istream &is)
        : m_is(is)
    {}

    void operator()(glm_type auto &field)
    {
        for (decltype(field.length()) i = 0; i < field.length(); i++)
            (*this)(field[i]);
    }

    void operator()(dynamic_array auto &field)
    {
        using size_type = decltype(field.size());
        size_type size;
        m_is.read(reinterpret_cast<char*>(&size), sizeof(size));
        LOG_SERIALIZATION("Dynamic aggregate of size: " << size);
        field.resize(size);
        for (size_type i = 0; i < size; i++)
        {
            (*this)(field[i]);
        }
    }

    template <class ElementType, typename SizeType, SizeType size>
    void operator()(std::array<ElementType, size> &field)
    {
        LOG_SERIALIZATION("Unserializing std::array of size: " << size);
        for (SizeType i = 0; i < size; i++)
        {
            (*this)(field[i]);
        }
    }

    template <typename T>
    requires std::is_arithmetic_v<T> || std::is_enum_v<T>
    void operator()(T &field)
    {
        m_is.read(reinterpret_cast<char*>(&field), sizeof(field));
        LOG_SERIALIZATION(field);
    }


    template <typename T>
    requires std::is_aggregate_v<T>
    void operator()(T &object) {
        boost::pfr::for_each_field(object, [this](auto &field) {
            (*this)(field);
        });
    }

private:
    std::istream &m_is;
};

} // namespace paca::fileformats
