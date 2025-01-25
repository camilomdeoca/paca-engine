#pragma once

#include "YamlSerialization.hpp"

#include <flecs.h>

#include <string>

namespace engine::serializers {

class FlecsSerializer
{
public:
    FlecsSerializer(const std::string &path)
        : m_serializer(path)
    {}

    void operator()(flecs::world &world);

    void write() { m_serializer.write(); }

private:
    engine::serializers::YamlSerializer m_serializer;
};

class FlecsUnserializer
{
public:
    FlecsUnserializer(const std::string &path)
        : m_unserializer(path)
    {}

    void operator()(flecs::world &world);

private:
    engine::serializers::YamlUnserializer m_unserializer;
};

} // namespace engine::serializers
