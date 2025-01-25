#pragma once

#include "opengl/Texture.hpp"

#include <string>

struct AnimatedMeshMetadata
{
    std::string name;
    std::string path;
    Texture preview;
};
