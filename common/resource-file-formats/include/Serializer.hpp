#pragma once

#include <glm/fwd.hpp>

#include <ostream>
#include <istream>
#include <vector>
#include <array>
#include <type_traits>

#ifdef LOG_EVERY_SERIALIZATION
    #include <iostream>
    #define LOG_SERIALIZATION(variable) { std::cout << variable << std::endl; }
#else
    #define LOG_SERIALIZATION(variable)
#endif

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

namespace paca::fileformats {

struct Bone;
struct Skeleton;
struct Mesh;
struct Model;
struct PositionKeyFrame;
struct RotationKeyFrame;
struct ScaleKeyFrame;
struct BoneKeyFrames;
struct Animation;
struct Texture;
struct Material;
struct ResourcePack;

class Serializer
{
public:
    Serializer(std::ostream &os)
        : m_os(os)
    {}

    template <dynamic_array T>
    void operator()(const T &field)
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

    void operator()(const auto &object) { static_assert(false, "The Serializer class needs regeneration."); }

    void operator()(const glm::vec3 &object);
    void operator()(const glm::mat4 &object);
    void operator()(const glm::quat &object);
    void operator()(const paca::fileformats::Bone &object);
    void operator()(const paca::fileformats::Skeleton &object);
    void operator()(const paca::fileformats::Mesh &object);
    void operator()(const paca::fileformats::Model &object);
    void operator()(const paca::fileformats::PositionKeyFrame &object);
    void operator()(const paca::fileformats::RotationKeyFrame &object);
    void operator()(const paca::fileformats::ScaleKeyFrame &object);
    void operator()(const paca::fileformats::BoneKeyFrames &object);
    void operator()(const paca::fileformats::Animation &object);
    void operator()(const paca::fileformats::Texture &object);
    void operator()(const paca::fileformats::Material &object);
    void operator()(const paca::fileformats::ResourcePack &object);

private:
    std::ostream &m_os;
};

class Unserializer
{
public:
    Unserializer(std::istream &is)
        : m_is(is)
    {}

    template <dynamic_array T>
    void operator()(T &field)
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

    void operator()(auto &object) { static_assert(false, "The Unserializer class needs regeneration."); }

    void operator()(glm::vec3 &object);
    void operator()(glm::mat4 &object);
    void operator()(glm::quat &object);
    void operator()(paca::fileformats::Bone &object);
    void operator()(paca::fileformats::Skeleton &object);
    void operator()(paca::fileformats::Mesh &object);
    void operator()(paca::fileformats::Model &object);
    void operator()(paca::fileformats::PositionKeyFrame &object);
    void operator()(paca::fileformats::RotationKeyFrame &object);
    void operator()(paca::fileformats::ScaleKeyFrame &object);
    void operator()(paca::fileformats::BoneKeyFrames &object);
    void operator()(paca::fileformats::Animation &object);
    void operator()(paca::fileformats::Texture &object);
    void operator()(paca::fileformats::Material &object);
    void operator()(paca::fileformats::ResourcePack &object);

private:
    std::istream &m_is;
};

} // namespace paca::fileformats
