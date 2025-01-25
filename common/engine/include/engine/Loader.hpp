#pragma once

#include <ResourceFileFormats.hpp>
#include <optional>

namespace engine::loaders {

template<typename T>
std::optional<T> load(const char *path);

template<>
std::optional<paca::fileformats::StaticMesh> load<paca::fileformats::StaticMesh>(const char *path);

template<>
std::optional<paca::fileformats::AnimatedMesh> load<paca::fileformats::AnimatedMesh>(const char *path);

template<>
std::optional<paca::fileformats::Animation> load<paca::fileformats::Animation>(const char *path);

template<>
std::optional<paca::fileformats::Texture> load<paca::fileformats::Texture>(const char *path);

template<>
std::optional<paca::fileformats::CubeMap> load<paca::fileformats::CubeMap>(const char *path);

}
