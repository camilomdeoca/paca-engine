#pragma once

#include "opengl/Texture.hpp"

#include <string>

struct StaticMeshMetadata
{
    std::string name;
    std::string path;
    Texture preview;
};
