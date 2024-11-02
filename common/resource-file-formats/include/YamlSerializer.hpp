#pragma once

#include "Serializer.hpp"
#include "utils/Assert.hpp"

#include <iterator>
#include <utils/Log.hpp>

#include <boost/pfr/core_name.hpp>
#include <glm/glm.hpp>

#include <ostream>
#include <array>
#include <type_traits>

#include <boost/pfr.hpp>
#include <yaml-cpp/emitter.h>
#include <yaml-cpp/emittermanip.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

namespace paca::fileformats {

class YamlSerializer
{
public:
    YamlSerializer(std::ostream &os)
        : m_os(os)
    {}

    ~YamlSerializer()
    {
        m_os << m_emitter.c_str();
    }

    void operator()(const glm_type auto &field)
    {
        m_emitter << YAML::Flow << YAML::BeginSeq;
        for (decltype(field.length()) i = 0; i < field.length(); i++)
            (*this)(field[i]);
        m_emitter << YAML::EndSeq;
    }

    template <typename T>
    requires is_specialization<T, std::vector>::value
    void operator()(const T &field)
    {
        using size_type = decltype(field.size());
        const size_type size = field.size();

        LOG_SERIALIZATION("Dynamic aggregate of size: " << size);
        m_emitter << YAML::BeginSeq;
        for (size_type i = 0; i < size; i++)
        {
            (*this)(field[i]);
        }
        m_emitter << YAML::EndSeq;
    }

    template <class ElementType, typename SizeType, SizeType size>
    void operator()(const std::array<ElementType, size> &field)
    {
        LOG_SERIALIZATION("Serializing std::array of size: " << size);
        m_emitter << YAML::BeginSeq;
        for (SizeType i = 0; i < size; i++)
        {
            (*this)(field[i]);
        }
        m_emitter << YAML::EndSeq;
    }

    template <typename T>
    requires std::is_arithmetic_v<T> || is_specialization<T, std::basic_string>::value
    void operator()(const T &field)
    {
        LOG_SERIALIZATION(field);
        m_emitter << field;
    }

    template <typename T>
    requires std::is_enum_v<T>
    void operator()(const T &field)
    {
        using UnderliyingType = std::underlying_type_t<T>;
        LOG_SERIALIZATION(field);
        m_emitter << static_cast<UnderliyingType>(field);
    }

    template <typename T>
    requires std::is_aggregate_v<T>
    void operator()(const T &object) {
        m_emitter << YAML::BeginMap;
        boost::pfr::for_each_field(object, [this](auto &field, std::size_t i) {
            m_emitter << YAML::Key << std::string(boost::pfr::names_as_array<T>()[i]);
            m_emitter << YAML::Value;
            (*this)(field);
        });
        m_emitter << YAML::EndMap;
    }

private:
    YAML::Emitter m_emitter;
    std::ostream &m_os;
};

class YamlUnserializer
{
public:
    YamlUnserializer(std::istream &is)
    {
        m_rootNode = YAML::Load(std::string(std::istreambuf_iterator<char>(is), {}));
    }

    ~YamlUnserializer()
    {
    }

    void operator()(glm_type auto &field, YAML::Node node = {})
    {
        if (node.IsNull()) node = m_rootNode;

        ASSERT(static_cast<decltype(node.size())>(field.length()) == node.size());
        for (decltype(field.length()) i = 0; i < field.length(); i++)
            (*this)(field[i], node[i]);
    }

    template <typename T>
    requires is_specialization<T, std::vector>::value
    void operator()(T &field, YAML::Node node = {})
    {
        if (node.IsNull()) node = m_rootNode;

        using size_type = decltype(field.size());
        size_type size = node.size();
        LOG_SERIALIZATION("Dynamic aggregate of size: " << size);
        field.resize(size);
        for (size_type i = 0; i < size; i++)
        {
            (*this)(field[i], node[i]);
        }
    }

    template <class ElementType, typename SizeType, SizeType size>
    void operator()(std::array<ElementType, size> &field, YAML::Node node = {})
    {
        if (node.IsNull()) node = m_rootNode;

        LOG_SERIALIZATION("Unserializing std::array of size: " << size);
        ASSERT(field.size() == node.size());
        for (SizeType i = 0; i < size; i++)
        {
            (*this)(field[i], node[i]);
        }
    }

    template <typename T>
    requires std::is_arithmetic_v<T> || is_specialization<T, std::basic_string>::value
    void operator()(T &field, YAML::Node node = {})
    {
        if (node.IsNull()) node = m_rootNode;

        ASSERT(node.IsScalar());
        field = node.as<T>();
        LOG_SERIALIZATION(field);
    }

    template <typename T>
    requires std::is_enum_v<T>
    void operator()(T &field, YAML::Node node = {})
    {
        if (node.IsNull()) node = m_rootNode;

        using UnderliyingType = std::underlying_type_t<T>;
        ASSERT(node.IsScalar());
        field = node.as<UnderliyingType>();
        LOG_SERIALIZATION(field);
    }

    template <typename T>
    requires std::is_aggregate_v<T>
    void operator()(T &object, YAML::Node node = {}) {
        if (node.IsNull()) node = m_rootNode;

        boost::pfr::for_each_field(object, [this, &node](auto &field, std::size_t i) {
            auto fieldName = boost::pfr::names_as_array<T>()[i];

            (*this)(field, node[std::string(fieldName)]);
        });
    }

private:
    YAML::Node m_rootNode;
};


} // namespace paca::fileformats
