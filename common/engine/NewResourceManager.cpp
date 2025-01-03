#include "engine/NewResourceManager.hpp"

#include "engine/StaticMesh.hpp"
#include "engine/AnimatedMesh.hpp"
#include "opengl/Texture.hpp"
#include "engine/Material.hpp"
#include "engine/Animation.hpp"
#include "engine/Font.hpp"
#include "serializers/BinarySerialization.hpp"

NewResourceManager::NewResourceManager()
{
    // For now until i make move costructors
    m_staticMeshes.reserve(100);
    m_animatedMeshes.reserve(100);
    m_textures.reserve(100);
    m_cubeMaps.reserve(100);
    m_materials.reserve(100);
    m_animations.reserve(100);
    m_fonts.reserve(100);
}

void NewResourceManager::loadAssetPack(const std::string &path)
{
    paca::fileformats::AssetPack assetPack;

    {
        serialization::BinaryUnserializer unserializer(path);
        unserializer(assetPack);
    }

    loadAssetPack(assetPack);
}

void NewResourceManager::loadAssetPack(paca::fileformats::AssetPack &assetPack)
{
    for (StaticMeshId id = 0; id < assetPack.staticMeshes.size(); id++)
    {
        paca::fileformats::StaticMesh &staticMesh = assetPack.staticMeshes[id];
        ASSERT(id == staticMesh.id);
        addStaticMesh(staticMesh);
    }

    for (AnimatedMeshId id = 0; id < assetPack.animatedMeshes.size(); id++)
    {
        paca::fileformats::AnimatedMesh &animatedMesh = assetPack.animatedMeshes[id];
        ASSERT(id == animatedMesh.id);
        addAnimatedMesh(animatedMesh);
    }

    for (TextureId id = 0; id < assetPack.textures.size(); id++)
    {
        paca::fileformats::Texture &texture = assetPack.textures[id];
        ASSERT(id == texture.id);
        addTexture(texture);
    }

    for (CubeMapId id = 0; id < assetPack.cubeMaps.size(); id++)
    {
        paca::fileformats::CubeMap &texture = assetPack.cubeMaps[id];
        ASSERT(id == texture.id);
        addCubeMap(texture);
    }

    for (MaterialId id = 0; id < assetPack.materials.size(); id++)
    {
        paca::fileformats::Material &material = assetPack.materials[id];
        ASSERT(id == material.id);
        addMaterial(material);
    }

    for (AnimationId id = 0; id < assetPack.animations.size(); id++)
    {
        paca::fileformats::Animation &animation = assetPack.animations[id];
        ASSERT(id == animation.id);
        addAnimation(animation);
    }

    for (FontId id = 0; id < assetPack.fonts.size(); id++)
    {
        paca::fileformats::Font &font = assetPack.fonts[id];
        ASSERT(id == font.id);
        addFont(font);
    }
}

const StaticMesh &NewResourceManager::getStaticMesh(StaticMeshId id) const
{
    ASSERT(id < m_staticMeshes.size());
    return m_staticMeshes[id];
}

const AnimatedMesh &NewResourceManager::getAnimatedMesh(AnimatedMeshId id) const
{
    ASSERT(id < m_animatedMeshes.size());
    return m_animatedMeshes[id];
}

const Texture &NewResourceManager::getTexture(TextureId id) const
{
    ASSERT(id < m_textures.size());
    return m_textures[id];
}

const Texture &NewResourceManager::getCubeMap(CubeMapId id) const
{
    ASSERT(id < m_cubeMaps.size());
    return m_cubeMaps[id];
}

const Material &NewResourceManager::getMaterial(MaterialId id) const
{
    ASSERT(id < m_materials.size());
    return m_materials[id];
}

const Animation &NewResourceManager::getAnimation(AnimationId id) const
{
    ASSERT(id < m_animations.size());
    return m_animations[id];
}

const Font &NewResourceManager::getFont(FontId id) const
{
    ASSERT(id < m_fonts.size());
    return m_fonts[id];
}

void NewResourceManager::addStaticMesh(paca::fileformats::StaticMesh &staticMesh)
{
#ifdef DEBUG
    size_t vertexCount = 0;
    vertexCount += staticMesh.vertices.size() / paca::fileformats::StaticMesh::vertex_size;
#endif // DEBUG

    m_staticMeshes.emplace_back(
                staticMesh.vertices,
                staticMesh.indices);
    INFO("Model {} has {} vertices", staticMesh.name, vertexCount);
}

void NewResourceManager::addAnimatedMesh(paca::fileformats::AnimatedMesh &animatedMesh)
{
#ifdef DEBUG
    size_t vertexCount = 0;
    vertexCount += animatedMesh.vertices.size() / animatedMesh.vertex_size;
#endif // DEBUG
    m_animatedMeshes.emplace_back(
                animatedMesh.vertices,
                animatedMesh.indices,
                animatedMesh.animations,
                std::move(animatedMesh.skeleton));
    INFO("Model {} has {} vertices", animatedMesh.name, vertexCount);
}

void NewResourceManager::addTexture(paca::fileformats::Texture &texture)
{
    Texture::Format format;
    switch (texture.channels)
    {
        case 1: format = Texture::Format::G8; break;
        case 2: format = Texture::Format::GA8; break;
        case 3: format = Texture::Format::RGB8; break;
        case 4: format = Texture::Format::RGBA8; break;
    }



    m_textures.emplace_back(Texture::Specification{
        .data = reinterpret_cast<const uint8_t*>(texture.pixelData.data()),
        .width = texture.width,
        .height = texture.height,
        .format = format,
        .mipmapLevels = 8,
        .autoGenerateMipmapLevels = true,
        .linearMinification = true,
        .linearMagnification = true,
        .interpolateBetweenMipmapLevels = true,
    });
}

void NewResourceManager::addCubeMap(paca::fileformats::CubeMap &cubeMap)
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
    m_cubeMaps.emplace_back(Texture::CubeMapSpecification{
        .facesData = facesData,
        .width = cubeMap.width,
        .height = cubeMap.height,
        .format = format,
        .linearMinification = true,
        .linearMagnification = true,
    });
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

void NewResourceManager::addMaterial(paca::fileformats::Material &material)
{
    MaterialSpecification materialSpec;
    for (uint32_t i = paca::fileformats::TextureType::none; i < paca::fileformats::TextureType::last; i++)
    {
        for (TextureId texture : material.textures[i])
        {
            materialSpec.textures[pacaTextureTypeToMaterialTextureType(paca::fileformats::TextureType::Type(i))]
                .emplace_back(texture);
        }
    }

    m_materials.emplace_back(materialSpec);
}

void NewResourceManager::addAnimation(paca::fileformats::Animation &animation)
{
    m_animations.emplace_back(
        animation.duration,
        animation.ticksPerSecond,
        animation.keyframes);
}

void NewResourceManager::addFont(paca::fileformats::Font &font)
{
    m_fonts.emplace_back(font.atlasTextureId, font.fontHeight, font.glyphs);
}

