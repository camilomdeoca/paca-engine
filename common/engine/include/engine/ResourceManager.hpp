#pragma once

#include "engine/Animation.hpp"
#include "engine/Material.hpp"
#include "engine/Model.hpp"
#include "opengl/Texture.hpp"

#include <ResourceFileFormats.hpp>

#include <memory>
#include <string>

enum class ResourceTypes {
    model,
    material,
    animation,
    texture,

    last
};

class ResourceManager {
public:
    static void loadAssetPack(const std::string &path);

    static std::shared_ptr<Texture> getTexture(const std::string &name);
    static std::shared_ptr<Texture> getCubeMap(const std::string &name);
    static std::shared_ptr<Model> getModel(const std::string &name);
    static std::shared_ptr<Material> getMaterial(const std::string &name);
    static std::shared_ptr<Animation> getAnimation(const std::string &name);

private:
    static void addModel(paca::fileformats::Model &model);
    static void addMaterial(const paca::fileformats::Material &material);
    static void addAnimation(paca::fileformats::Animation &animation);
    static void addTexture(const paca::fileformats::Texture &texture);
};
