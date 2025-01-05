#pragma once

#include <utils/Log.hpp>

#include <glm/glm.hpp>

#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

#include <fstream>
#include <type_traits>
#include <fstream>
#include <variant>
#include <vector>

namespace engine::serializers {

namespace detail {

//! @internal
template<
    size_t N,
    typename... Types,
    class Unserializer,
    std::enable_if_t<N >= std::variant_size_v<std::variant<Types...>>, bool> = true>
void unserializeVariantFromName(
    Unserializer &unserializer,
    YAML::Node node,
    const std::string &name,
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
void unserializeVariantFromName(
    Unserializer &unserializer,
    YAML::Node node,
    const std::string &name,
    std::variant<Types...> &variant)
{
    if(std::variant_alternative_t<N, std::variant<Types...>>::getClassName() == name)
    {
        variant.template emplace<N>();
        unserializer(std::get<N>(variant), node);
    }
    else
    {
        unserializeVariantFromName<N+1>(unserializer, node, name, variant);
    }
}

}

class YamlSerializer {
public:
    YamlSerializer(const std::string &path)
        : m_path(path)
    {}


    void write()
    {
        std::ofstream ofs(m_path.c_str());
        ofs << m_out.c_str();
    }

    YAML::Emitter &getYamlEmitter() { return m_out; }

    template<typename T>
    void operator()(const char *name, const T &value)
    {
        m_out << YAML::BeginMap;
        m_out << YAML::Key << name;
        m_out << YAML::Value;
        (*this)(value);
        m_out << YAML::EndMap;
    }

    template<typename T>
    void operator()(const T &value)
    {
        if constexpr (std::is_enum_v<T>)
        {
            m_out << std::to_underlying(value);
        }
        else if constexpr (std::is_class_v<T>)
        {
            m_out << YAML::BeginMap;
            value.forEachFieldWithName([this](auto &field, std::string_view name) {
                m_out << YAML::Key << name.data();
                m_out << YAML::Value;
                (*this)(field);
            });
            m_out << YAML::EndMap;
        }
        else
        {
            m_out << value;
        }
    }

    void operator()(const std::string &value)
    {
        m_out << value;
    }

    template<typename T>
    void operator()(const std::vector<T> &value)
    {
        m_out << YAML::BeginSeq;
        for(const T &elem : value)
        {
            (*this)(elem);
        }
        m_out << YAML::EndSeq;
    }

    template<typename T, size_t Size>
    void operator()(const std::array<T, Size> &value)
    {
        for(const T &elem : value)
        {
            (*this)(elem);
        }
    }

    template<typename... Types>
    void operator()(const std::variant<Types...> &value)
    {
        m_out << YAML::BeginMap;
        std::visit([this](auto &value) {
            m_out << YAML::Key << value.getClassName().data();
            m_out << YAML::Value;
            (*this)(value);
        }, value);
        m_out << YAML::EndMap;
    }

    template<glm::length_t Length, typename T>
    void operator()(const glm::vec<Length, T> &value)
    {
        m_out << YAML::Flow << YAML::BeginSeq;
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this)(value[i]);
        }
        m_out << YAML::EndSeq;
    }

    template<glm::length_t Columns, glm::length_t Rows, typename T>
    void operator()(const glm::mat<Columns, Rows, T> &value, const char *name)
    {
        m_out << YAML::BeginSeq;
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this)(value[i]);
        }
        m_out << YAML::EndSeq;
    }

    template<typename T>
    void operator()(const glm::qua<T> &value, const char *name)
    {
        m_out << YAML::BeginSeq;
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this)(value[i]);
        }
        m_out << YAML::EndSeq;
    }

protected:
    std::string m_path;
    YAML::Emitter m_out;
};

class YamlUnserializer {
public:
    YamlUnserializer(const std::string &path)
        : m_root(YAML::LoadFile(path))
    {}

    YAML::Node getYamlNode() { return m_root; }

    template<typename T>
    void operator()(T &value, YAML::Node node)
    {
        if constexpr (std::is_enum_v<T>)
        {
            value = T(node.as<std::underlying_type_t<T>>());
        }
        else if constexpr (std::is_class_v<T>)
        {
            value.forEachFieldWithName([this, &node](auto &field, std::string_view name) {
                (*this)(field, node[name.data()]);
            });
        }
        else
        {
            value = node.as<T>();
        }
    }

    void operator()(std::string &value, YAML::Node node)
    {
        value = node.as<std::string>();
    }

    template<typename T>
    void operator()(std::vector<T> &value, YAML::Node node)
    {
        value.resize(node.size());
        for(size_t i = 0; i < value.size(); i++)
        {
            (*this)(value[i], node[i]);
        }
    }

    template<typename T, size_t Size>
    void operator()(std::array<T, Size> &value, YAML::Node node)
    {
        for(size_t i = 0; i < value.size(); i++)
        {
            T &elem = value[i];
            (*this)(elem, node[i]);
        }
    }

    template<typename... Types>
    void operator()(std::variant<Types...> &value, YAML::Node node)
    {
        std::string name = node.begin()->first.as<std::string>();
        detail::unserializeVariantFromName<0>(*this, node[name], name, value);
    }

    template<glm::length_t Length, typename T>
    void operator()(glm::vec<Length, T> &value, YAML::Node node)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this)(value[i], node[i]);
        }
    }

    template<glm::length_t Columns, glm::length_t Rows, typename T>
    void operator()(glm::mat<Columns, Rows, T> &value, YAML::Node node)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this)(value[i], node[i]);
        }
    }

    template<typename T>
    void operator()(glm::qua<T> &value, YAML::Node node)
    {
        for(glm::length_t i = 0; i < value.length(); i++)
        {
            (*this)(value[i], node[i]);
        }
    }

protected:
    YAML::Node m_root;
};

} // namespace engine::serializers

