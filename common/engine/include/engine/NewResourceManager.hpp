#pragma once

#include <ResourceFileFormats.hpp>

#include "StaticMesh.hpp"
#include "AnimatedMesh.hpp"
#include "opengl/Texture.hpp"
#include "Material.hpp"
#include "Animation.hpp"
#include "Font.hpp"

#include <string>

class NewResourceManager
{
public:
    NewResourceManager();

    void loadAssetPack(const std::string &path);
    void loadAssetPack(paca::fileformats::AssetPack &assetPack);

    const StaticMesh &getStaticMesh(StaticMeshId id) const;
    const AnimatedMesh &getAnimatedMesh(AnimatedMeshId id) const;
    const Texture &getTexture(TextureId id) const;
    const Texture &getCubeMap(CubeMapId id) const;
    const Material &getMaterial(MaterialId id) const;
    const Animation &getAnimation(AnimationId id) const;
    const Font &getFont(FontId id) const;

private:
    void addStaticMesh(paca::fileformats::StaticMesh &staticMesh);
    void addAnimatedMesh(paca::fileformats::AnimatedMesh &animatedMesh);
    void addTexture(paca::fileformats::Texture &texture);
    void addCubeMap(paca::fileformats::CubeMap &cubeMap);
    void addMaterial(paca::fileformats::Material &material);
    void addAnimation(paca::fileformats::Animation &animation);
    void addFont(paca::fileformats::Font &font);

    std::vector<StaticMesh> m_staticMeshes;
    std::vector<AnimatedMesh> m_animatedMeshes;
    std::vector<Texture> m_textures;
    std::vector<Texture> m_cubeMaps;
    std::vector<Material> m_materials;
    std::vector<Animation> m_animations;
    std::vector<Font> m_fonts;
};
