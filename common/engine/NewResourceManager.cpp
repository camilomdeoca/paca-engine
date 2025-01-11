#include "engine/NewResourceManager.hpp"

#include "engine/StaticMesh.hpp"
#include "engine/AnimatedMesh.hpp"
#include "opengl/Texture.hpp"
#include "engine/Material.hpp"
#include "engine/Animation.hpp"
#include "engine/Font.hpp"

#include <algorithm>

const StaticMesh *NewResourceManager::get(StaticMeshId id) const
{
    const auto it = m_staticMeshes.find(id);
    if (it != m_staticMeshes.end())
        return &it->second;
    return nullptr;
}

const AnimatedMesh *NewResourceManager::get(AnimatedMeshId id) const
{
    const auto it = m_animatedMeshes.find(id);
    if (it != m_animatedMeshes.end())
        return &it->second;
    return nullptr;
}

const Texture *NewResourceManager::get(TextureId id) const
{
    const auto it = m_textures.find(id);
    if (it != m_textures.end())
        return &it->second;
    return nullptr;
}

const Texture *NewResourceManager::get(CubeMapId id) const
{
    const auto it = m_cubemaps.find(id);
    if (it != m_cubemaps.end())
        return &it->second;
    return nullptr;
}

const Material *NewResourceManager::get(MaterialId id) const
{
    const auto it = m_materials.find(id);
    if (it != m_materials.end())
        return &it->second;
    return nullptr;
}

const Animation *NewResourceManager::get(AnimationId id) const
{
    const auto it = m_animations.find(id);
    if (it != m_animations.end())
        return &it->second;
    return nullptr;
}

const Font *NewResourceManager::get(FontId id) const
{
    const auto it = m_fonts.find(id);
    if (it != m_fonts.end())
        return &it->second;
    return nullptr;
}

bool NewResourceManager::remove(StaticMeshId id)
{
    return m_staticMeshes.erase(id);
}

bool NewResourceManager::remove(AnimatedMeshId id)
{
    return m_animatedMeshes.erase(id);
}

bool NewResourceManager::remove(TextureId id)
{
    return m_textures.erase(id);
}

bool NewResourceManager::remove(CubeMapId id)
{
    return m_cubemaps.erase(id);
}

bool NewResourceManager::remove(MaterialId id)
{
    return m_materials.erase(id);
}

bool NewResourceManager::remove(AnimationId id)
{
    return m_animations.erase(id);
}

bool NewResourceManager::remove(FontId id)
{
    return m_fonts.erase(id);
}


void NewResourceManager::add(paca::fileformats::StaticMesh &staticMesh)
{
#ifdef DEBUG
    size_t vertexCount = 0;
    vertexCount += staticMesh.vertices.size() / paca::fileformats::StaticMesh::vertex_size;
#endif // DEBUG

    const auto it = m_staticMeshes.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(StaticMeshId(staticMesh.id)),
        std::forward_as_tuple(
            staticMesh.vertices,
            staticMesh.indices));

    ASSERT_MSG(it.second, "Error static mesh id {} is already on assets", staticMesh.id);
    INFO("Model {} has {} vertices", staticMesh.name, vertexCount);
}

void NewResourceManager::add(paca::fileformats::AnimatedMesh &animatedMesh)
{
#ifdef DEBUG
    size_t vertexCount = 0;
    vertexCount += animatedMesh.vertices.size() / animatedMesh.vertex_size;
#endif // DEBUG
    
    std::vector<AnimationId> animations(animatedMesh.animations.size());
    std::transform(animatedMesh.animations.begin(), animatedMesh.animations.end(),
            animations.begin(), [](paca::fileformats::AnimationId id) { return AnimationId(id); });
    const auto it = m_animatedMeshes.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(AnimatedMeshId(animatedMesh.id)),
        std::forward_as_tuple(
            animatedMesh.vertices,
            animatedMesh.indices,
            animations,
            std::move(animatedMesh.skeleton)));

    ASSERT_MSG(it.second, "Error animated mesh id {} is already on assets", animatedMesh.id);
    INFO("Model {} has {} vertices", animatedMesh.name, vertexCount);
}

void NewResourceManager::add(paca::fileformats::Texture &texture)
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

void NewResourceManager::add(paca::fileformats::CubeMap &cubeMap)
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
            std::forward_as_tuple(Texture::CubeMapSpecification{
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

void NewResourceManager::add(paca::fileformats::Material &material)
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

void NewResourceManager::add(paca::fileformats::Animation &animation)
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

void NewResourceManager::add(paca::fileformats::Font &font)
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

