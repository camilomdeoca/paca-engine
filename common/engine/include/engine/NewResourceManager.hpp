#pragma once

#include <ResourceFileFormats.hpp>

#include "StaticMesh.hpp"
#include "AnimatedMesh.hpp"
#include "opengl/Texture.hpp"
#include "Material.hpp"
#include "Animation.hpp"
#include "Font.hpp"

class NewResourceManager
{
public:
    const StaticMesh *get(StaticMeshId id) const;
    const AnimatedMesh *get(AnimatedMeshId id) const;
    const Texture *get(TextureId id) const;
    const Texture *get(CubeMapId id) const;
    const Material *get(MaterialId id) const;
    const Animation *get(AnimationId id) const;
    const Font *get(FontId id) const;

    bool remove(StaticMeshId id);
    bool remove(AnimatedMeshId id);
    bool remove(TextureId id);
    bool remove(CubeMapId id);
    bool remove(MaterialId id);
    bool remove(AnimationId id);
    bool remove(FontId id);

    void add(paca::fileformats::StaticMesh &staticMesh);
    void add(paca::fileformats::AnimatedMesh &animatedMesh);
    void add(paca::fileformats::Texture &texture);
    void add(paca::fileformats::CubeMap &cubeMap);
    void add(paca::fileformats::Material &material);
    void add(paca::fileformats::Animation &animation);
    void add(paca::fileformats::Font &font);

    auto &staticMeshes() { return m_staticMeshes; }
    auto &animatedMeshes() { return m_animatedMeshes; }
    auto &textures() { return m_textures; }
    auto &cubemaps() { return m_cubemaps; }
    auto &materials() { return m_materials; }
    auto &animations() { return m_animations; }
    auto &fonts() { return m_fonts; }

private:

    // TODO: change unordered_map to a faster hashmap that has everything close in memory
    std::unordered_map<StaticMeshId,   StaticMesh>   m_staticMeshes;
    std::unordered_map<AnimatedMeshId, AnimatedMesh> m_animatedMeshes;
    std::unordered_map<TextureId,      Texture>      m_textures;
    std::unordered_map<CubeMapId,      Texture>      m_cubemaps;
    std::unordered_map<MaterialId,     Material>     m_materials;
    std::unordered_map<AnimationId,    Animation>    m_animations;
    std::unordered_map<FontId,         Font>         m_fonts;
};
