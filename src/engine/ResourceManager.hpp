#pragma once

#include "engine/Material.hpp"
#include "engine/Model.hpp"
#include "opengl/Texture.hpp"

#include <memory>

class ResourceManager {
public:
    static std::shared_ptr<Model> addModel(std::string_view path);
    static std::shared_ptr<Material> addMaterial(std::string_view path);

    static std::shared_ptr<Texture> getTexture(std::string_view path);
    static std::shared_ptr<Model> getModel(std::string_view name);
    static std::shared_ptr<Material> getMaterial(std::string_view name);
};
