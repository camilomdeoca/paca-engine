#include "engine/AssetManager.hpp"

#include "engine/assets/StaticMesh.hpp"
#include "engine/assets/AnimatedMesh.hpp"
#include "opengl/Texture.hpp"
#include "engine/assets/Material.hpp"
#include "engine/assets/Animation.hpp"
#include "engine/assets/Font.hpp"

const StaticMesh *AssetManager::get(StaticMeshId id) const
{
    const auto it = m_staticMeshes.find(id);
    if (it != m_staticMeshes.end())
        return &it->second;
    return nullptr;
}

const AnimatedMesh *AssetManager::get(AnimatedMeshId id) const
{
    const auto it = m_animatedMeshes.find(id);
    if (it != m_animatedMeshes.end())
        return &it->second;
    return nullptr;
}

const Texture *AssetManager::get(TextureId id) const
{
    const auto it = m_textures.find(id);
    if (it != m_textures.end())
        return &it->second;
    return nullptr;
}

const Cubemap *AssetManager::get(CubeMapId id) const
{
    const auto it = m_cubemaps.find(id);
    if (it != m_cubemaps.end())
        return &it->second;
    return nullptr;
}

const Material *AssetManager::get(MaterialId id) const
{
    const auto it = m_materials.find(id);
    if (it != m_materials.end())
        return &it->second;
    return nullptr;
}

const Animation *AssetManager::get(AnimationId id) const
{
    const auto it = m_animations.find(id);
    if (it != m_animations.end())
        return &it->second;
    return nullptr;
}

const Font *AssetManager::get(FontId id) const
{
    const auto it = m_fonts.find(id);
    if (it != m_fonts.end())
        return &it->second;
    return nullptr;
}

bool AssetManager::remove(StaticMeshId id)
{
    return m_staticMeshes.erase(id);
}

bool AssetManager::remove(AnimatedMeshId id)
{
    return m_animatedMeshes.erase(id);
}

bool AssetManager::remove(TextureId id)
{
    return m_textures.erase(id);
}

bool AssetManager::remove(CubeMapId id)
{
    return m_cubemaps.erase(id);
}

bool AssetManager::remove(MaterialId id)
{
    return m_materials.erase(id);
}

bool AssetManager::remove(AnimationId id)
{
    return m_animations.erase(id);
}

bool AssetManager::remove(FontId id)
{
    return m_fonts.erase(id);
}

bool AssetManager::move(StaticMeshId from, StaticMeshId to)
{
    if (m_staticMeshes.contains(to)) return false;
    auto node = m_staticMeshes.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_staticMeshes.insert(std::move(node)).inserted;
}

bool AssetManager::move(AnimatedMeshId from, AnimatedMeshId to)
{
    if (m_animatedMeshes.contains(to)) return false;
    auto node = m_animatedMeshes.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_animatedMeshes.insert(std::move(node)).inserted;
}

bool AssetManager::move(TextureId from, TextureId to)
{
    if (m_textures.contains(to)) return false;
    auto node = m_textures.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_textures.insert(std::move(node)).inserted;
}

bool AssetManager::move(CubeMapId from, CubeMapId to)
{
    if (m_cubemaps.contains(to)) return false;
    auto node = m_cubemaps.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_cubemaps.insert(std::move(node)).inserted;
}

bool AssetManager::move(MaterialId from, MaterialId to)
{
    if (m_materials.contains(to)) return false;
    auto node = m_materials.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_materials.insert(std::move(node)).inserted;
}

bool AssetManager::move(AnimationId from, AnimationId to)
{
    if (m_animations.contains(to)) return false;
    auto node = m_animations.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_animations.insert(std::move(node)).inserted;
}

bool AssetManager::move(FontId from, FontId to)
{
    if (m_fonts.contains(to)) return false;
    auto node = m_fonts.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_fonts.insert(std::move(node)).inserted;
}


void AssetManager::add(paca::fileformats::StaticMesh &staticMesh)
{
    const auto it = m_staticMeshes.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(StaticMeshId(staticMesh.id)),
        std::forward_as_tuple(
            std::span
            {
                reinterpret_cast<StaticMesh::Vertex*>(staticMesh.vertices.data()),
                staticMesh.vertices.size()
            },
            std::span
            {
                staticMesh.indices
            },
            AxisAlignedBoundingBox{staticMesh.aabb.min, staticMesh.aabb.max}));

    ASSERT_MSG(it.second, "Error static mesh id {} is already on assets", staticMesh.id);
}

void AssetManager::add(paca::fileformats::AnimatedMesh &animatedMesh)
{
    const auto it = m_animatedMeshes.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(AnimatedMeshId(animatedMesh.id)),
        std::forward_as_tuple(
            std::span
            {
                reinterpret_cast<AnimatedMesh::Vertex*>(animatedMesh.vertices.data()),
                animatedMesh.vertices.size()
            },
            std::span
            {
                animatedMesh.indices
            },
            std::move(animatedMesh.skeleton)));

    ASSERT_MSG(it.second, "Error animated mesh id {} is already on assets", animatedMesh.id);
}

void AssetManager::add(paca::fileformats::Texture &texture)
{
    Texture::Format format;
    switch (texture.channels)
    {
        case 1: format = Texture::Format::G8; break;
        case 2: format = Texture::Format::GA8; break;
        case 3: format = Texture::Format::RGB8; break;
        case 4: format = Texture::Format::RGBA8; break;
    }



    const auto it = m_textures.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(TextureId(texture.id)),
            std::forward_as_tuple(Texture::Specification{
                .data = reinterpret_cast<const uint8_t*>(texture.pixelData.data()),
                .width = texture.width,
                .height = texture.height,
                .format = format,
                .mipmapLevels = 8,
                .autoGenerateMipmapLevels = true,
                .linearMinification = true,
                .linearMagnification = true,
                .interpolateBetweenMipmapLevels = true,
            }));

    ASSERT_MSG(it.second, "Error texture id {} is already on assets", texture.id);
}

void AssetManager::add(paca::fileformats::CubeMap &cubeMap)
{
    Texture::Format format;
    switch (cubeMap.channels)
    {
        case 1: format = Texture::Format::G8; break;
        case 2: format = Texture::Format::GA8; break;
        case 3: format = Texture::Format::RGB8; break;
        case 4: format = Texture::Format::RGBA8; break;
    }

    std::array<const unsigned char*, 6> facesData;
    for (unsigned int i = 0; i < facesData.size(); i++)
    {
        facesData[i] = reinterpret_cast<const uint8_t*>(
            cubeMap.pixelData.data()
            + cubeMap.width * cubeMap.height * cubeMap.channels * i);
    }

    const auto it = m_cubemaps.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(CubeMapId(cubeMap.id)),
            std::forward_as_tuple(Cubemap::Specification{
                .facesData = facesData,
                .width = cubeMap.width,
                .height = cubeMap.height,
                .format = format,
                .linearMinification = true,
                .linearMagnification = true,
            }));

    ASSERT_MSG(it.second, "Error texture id {} is already on assets", cubeMap.id);
}

MaterialTextureType::Type pacaTextureTypeToMaterialTextureType(paca::fileformats::TextureType::Type type)
{
    switch (type) {
        case paca::fileformats::TextureType::diffuse:  return MaterialTextureType::diffuse;
        case paca::fileformats::TextureType::specular: return MaterialTextureType::specular;
        case paca::fileformats::TextureType::normal:   return MaterialTextureType::normal;
        case paca::fileformats::TextureType::depth:    return MaterialTextureType::height;
        default: break;
    }

    ASSERT_MSG(false, "Invalid paca texture type!");
}

void AssetManager::add(paca::fileformats::Material &material)
{
    MaterialSpecification materialSpec;
    for (uint32_t i = paca::fileformats::TextureType::none; i < paca::fileformats::TextureType::last; i++)
    {
        for (uint32_t texture : material.textures[i])
        {
            materialSpec.textures[pacaTextureTypeToMaterialTextureType(paca::fileformats::TextureType::Type(i))]
                .emplace_back(TextureId(texture));
        }
    }

    const auto it = m_materials.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(MaterialId(material.id)),
        std::forward_as_tuple(materialSpec));

    ASSERT_MSG(it.second, "Error material id {} is already on assets", material.id);
}

void AssetManager::add(paca::fileformats::Animation &animation)
{
    const auto it = m_animations.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(AnimationId(animation.id)),
        std::forward_as_tuple(
            animation.duration,
            animation.ticksPerSecond,
            animation.keyframes));
    ASSERT_MSG(it.second, "Error animation id {} is already on assets", animation.id);
}

void AssetManager::add(paca::fileformats::Font &font)
{
    const auto it = m_fonts.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(FontId(font.id)),
        std::forward_as_tuple(
            TextureId(font.atlasTextureId),
            font.fontHeight,
            font.glyphs));

    ASSERT_MSG(it.second, "Error font id {} is already on assets", font.id);
}

