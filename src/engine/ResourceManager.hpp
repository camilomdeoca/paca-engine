#pragma once

#include "opengl/Texture.hpp"

#include <memory>

class ResourceManager {
public:
    static std::shared_ptr<Texture> getTexture(const std::string &path);
};
