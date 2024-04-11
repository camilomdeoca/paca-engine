#pragma once

#include "engine/Material.hpp"
#include "engine/Model.hpp"
#include "opengl/Texture.hpp"

#include <memory>
#include <string>

class ResourceManager {
public:
    static std::shared_ptr<Model> addModel(const std::string &path);
    static std::shared_ptr<Material> addMaterial(const std::string &path);

    static std::shared_ptr<Texture> getTexture(const std::string &path);
    static std::shared_ptr<Texture> getCubeMap(const std::string &folder);
    static std::shared_ptr<Model> getModel(const std::string &name);
    static std::shared_ptr<Material> getMaterial(const std::string &name);
};
