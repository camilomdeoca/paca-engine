#pragma once

#include "utils/Log.hpp"

#include <glm/fwd.hpp>

#include <fstream>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace serialization {

namespace detail {

//! @internal
template<
    size_t N,
    typename... Types,
    class Unserializer,
    std::enable_if_t<N >= std::variant_size_v<std::variant<Types...>>, bool> = true>
void unserializeVariant(
    Unserializer &unserializer,
    size_t targetIndex,
    std::variant<Types...> &variant)
{
    FATAL("Invalid type index for variant");
}

//! @internal
template<
    size_t N,
    typename... Types,
    class Unserializer,
    std::enable_if_t<N < std::variant_size_v<std::variant<Types...>>, bool> = true> 
void unserializeVariant(
    Unserializer &unserializer,
    size_t targetIndex,
    std::variant<Types...> &variant)
{
    if(N == targetIndex)
    {
        variant.template emplace<N>();
        unserializer << std::get<N>(variant);
    }
    else
    {
        unserializeVariant<N+1>(unserializer, targetIndex, variant);
    }
}

}

template<typename Writer>
class Serializer {
public:
    template<typename T>
    Serializer &operator<<(const T &value)
    {
        if constexpr (std::is_enum_v<T>)
        {
            m_writer.write(std::to_underlying(value));
        }
        else if constexpr (std::is_class_v<T>)
        {
            value.forEachField(*this);
        }
        else
        {
            m_writer.write(value);
        }
        return *this;
    }

    Serializer &operator<<(const std::string &value)
    {
        m_writer.write(value.size());
        for(const char &elem : value)
        {
            (*this) << elem;
        }
        return *this;
    }

    template<typename T>
    Serializer &operator<<(const std::vector<T> &value)
    {
        m_writer.write(value.size());
        for(const T &elem : value)
        {
            (*this) << elem;
        }
        return *this;
    }

    template<typename T, size_t Size>
    Serializer &operator<<(const std::array<T, Size> &value)
    {
        for(const T &elem : value)
        {
            (*this) << elem;
        }
        return *this;
    }

    template<typename... Types>
    Serializer &operator<<(const std::variant<Types...> &value)
    {
        m_writer.write(value.index());
        std::visit([this](auto value) { (*this) << value; }, value);
        return *this;
    }

    template<glm::length_t Length, typename T>
    Serializer &operator<<(const glm::vec<Length, T> &value)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this) << value[i];
        }
        return *this;
    }

    template<glm::length_t Columns, glm::length_t Rows, typename T>
    Serializer &operator<<(const glm::mat<Columns, Rows, T> &value)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this) << value[i];
        }
        return *this;
    }

    template<typename T>
    Serializer &operator<<(const glm::qua<T> &value)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this) << value[i];
        }
        return *this;
    }

    template<typename ...Args>
    Serializer &operator()(const Args &...values)
    {
        ((*this) << ... << values);
        return *this;
    }
protected:
    Writer m_writer;
};

template<typename Reader>
class Unserializer {
public:
    template<typename T>
    Unserializer &operator<<(T &value)
    {
        if constexpr (std::is_enum_v<T>)
        {
            m_reader.read(std::to_underlying(value));
        }
        else if constexpr (std::is_class_v<T>)
        {
            value.forEachField(*this);
        }
        else
        {
            m_reader.read(value);
        }
        return *this;
    }

    Unserializer &operator<<(std::string &value)
    {
        decltype(value.size()) size = 0;
        m_reader.read(size);
        value.resize(size);
        for(char &elem : value)
        {
            (*this) << elem;
        }
        return *this;
    }

    template<typename T>
    Unserializer &operator<<(std::vector<T> &value)
    {
        decltype(value.size()) size = 0;
        m_reader.read(size);
        value.resize(size);
        for(T &elem : value)
        {
            (*this) << elem;
        }
        return *this;
    }

    template<typename T, size_t Size>
    Unserializer &operator<<(std::array<T, Size> &value)
    {
        for(T &elem : value)
        {
            (*this) << elem;
        }
        return *this;
    }

    template<typename... Types>
    Unserializer &operator<<(std::variant<Types...> &value)
    {
        decltype(value.index()) index = std::variant_npos;
        m_reader.read(index);
        detail::unserializeVariant<0>(*this, index, value);
        return *this;
    }

    template<glm::length_t Length, typename T>
    Unserializer &operator<<(glm::vec<Length, T> &value)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this) << value[i];
        }
        return *this;
    }

    template<glm::length_t Columns, glm::length_t Rows, typename T>
    Unserializer &operator<<(glm::mat<Columns, Rows, T> &value)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this) << value[i];
        }
        return *this;
    }

    template<typename T>
    Unserializer &operator<<(glm::qua<T> &value)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this) << value[i];
        }
        return *this;
    }

    template<typename ...Args>
    Unserializer &operator()(Args &...values)
    {
        ((*this) << ... << values);
        return *this;
    }

protected:
    Reader m_reader;
};

class BinaryWriter {
public:
    bool open(const std::string &path)
    {
        m_ofs.open(path);
        bool isGood = m_ofs.good();
        if (!isGood) WARN("error opening file: {}.", path);
        return isGood;
    }

    template<typename T>
    requires std::is_arithmetic_v<T>
    void write(const T &v)
    {
        m_ofs.write(reinterpret_cast<const char*>(&v), sizeof(v));
    }

private:
    std::ofstream m_ofs;
};

class BinaryReader {
public:
    bool open(const std::string &path)
    {
        m_ifs.open(path);
        bool isGood = m_ifs.good();
        if (!isGood) WARN("error opening file: {}.", path);
        return m_ifs.good();
    }

    template<typename T>
    requires std::is_arithmetic_v<T>
    void read(T &v)
    {
        m_ifs.read(reinterpret_cast<char*>(&v), sizeof(v));
    }

private:
    std::ifstream m_ifs;
};

class BinarySerializer : public Serializer<BinaryWriter>
{
public:
    BinarySerializer(const std::string &path)
    {
        m_writer.open(path);
    }
};

class BinaryUnserializer : public Unserializer<BinaryReader>
{
public:
    BinaryUnserializer(const std::string &path)
    {
        m_reader.open(path);
    }
};

} // namespace serialization

