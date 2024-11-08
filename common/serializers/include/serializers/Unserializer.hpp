#pragma once

#include "Concepts.hpp"
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
