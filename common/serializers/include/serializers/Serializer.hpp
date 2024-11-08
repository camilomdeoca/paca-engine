#pragma once

#include "Concepts.hpp"

#include <ostream>
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

} // namespace paca::fileformats
