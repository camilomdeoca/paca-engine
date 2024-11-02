#include "engine/ResourceManager.hpp"

#include "engine/AnimatedMesh.hpp"
#include "utils/Assert.hpp"
#include "engine/Material.hpp"
#include "engine/Mesh.hpp"
#include "engine/Model.hpp"
#include "engine/StaticMesh.hpp"
#include "opengl/Texture.hpp"

#include <ResourceFileFormats.hpp>
#include <Serializer.hpp>

#include <functional>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <fstream>

struct StringAndViewHash : public std::hash<std::string_view> { using is_transparent = void; };

static struct {
    std::unordered_map<std::string, std::shared_ptr<Texture>, StringAndViewHash, std::equal_to<>> textures;
    std::unordered_map<std::string, std::shared_ptr<Texture>, StringAndViewHash, std::equal_to<>> cubeMaps;

    // TODO: Change to weak_ptr so the materials and models get deleted when not used.
    // TODO: Add an index file of in wich files are what materials so when the materials are loaded
    // when wanted.
    std::unordered_map<std::string, std::shared_ptr<Material>, StringAndViewHash, std::equal_to<>> materials;
    std::unordered_map<std::string, std::shared_ptr<Model>, StringAndViewHash, std::equal_to<>> models; 
    std::unordered_map<std::string, std::shared_ptr<Animation>, StringAndViewHash, std::equal_to<>> animations; 
} s_data;

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

void ResourceManager::loadAssetPack(const std::string &path)
{
    std::ifstream ifs(path);
    paca::fileformats::Unserializer unserializer(ifs);
    paca::fileformats::AssetPack pack;
    unserializer(pack);

    for (paca::fileformats::Texture &texture : pack.textures)
        addTexture(texture);
    for (paca::fileformats::Material &material : pack.materials)
        addMaterial(material);
    for (paca::fileformats::Animation &animation : pack.animations)
        addAnimation(animation);
    for (paca::fileformats::Model &model : pack.models)
        addModel(model);
}

void ResourceManager::addModel(paca::fileformats::Model &model)
{
    std::vector<std::shared_ptr<Mesh>> meshes;

#ifdef DEBUG
    size_t vertexCount = 0;
#endif // DEBUG

    for (paca::fileformats::Mesh &pacaMesh : model.meshes)
    {
#ifdef DEBUG
        vertexCount += pacaMesh.vertices.size() / paca::fileformats::vertexTypeToSize(pacaMesh.vertexType);
#endif // DEBUG
        
        std::vector<std::shared_ptr<Animation>> animations;
        for (const std::string &animName : pacaMesh.animations)
            animations.emplace_back(getAnimation(animName));
        // if mesh is animated
        if (pacaMesh.vertexType == paca::fileformats::VertexType::float3pos_float3norm_float3tang_float2texture_int4boneIds_float4boneWeights)
            meshes.emplace_back(std::make_shared<AnimatedMesh>(
                        pacaMesh.vertices,
                        pacaMesh.indices,
                        getMaterial(pacaMesh.materialName),
                        animations,
                        std::move(pacaMesh.skeleton)));
        // else (mesh is static)
        else
            meshes.emplace_back(std::make_shared<StaticMesh>(
                        pacaMesh.vertices,
                        pacaMesh.indices,
                        getMaterial(pacaMesh.materialName)));
    }
    INFO("Model {} has {} vertices", model.name, vertexCount);

    std::shared_ptr<Model> outModel = std::make_shared<Model>(meshes);
    s_data.models.insert(std::make_pair(model.name, outModel));
}

void ResourceManager::addMaterial(const paca::fileformats::Material &material)
{
    MaterialSpecification materialSpec;
    for (uint32_t i = paca::fileformats::TextureType::none; i < paca::fileformats::TextureType::last; i++)
    {
        for (const std::string &texture : material.textures[i])
        {
            materialSpec.textureMaps[pacaTextureTypeToMaterialTextureType(paca::fileformats::TextureType::Type(i))]
                .emplace_back(getTexture(texture));
        }
    }

    std::shared_ptr<Material> outMaterial = std::make_shared<Material>(materialSpec);
    s_data.materials.insert(std::make_pair(material.name, outMaterial));
}

void ResourceManager::addAnimation(paca::fileformats::Animation &animation)
{
    std::shared_ptr<Animation> outAnimation = std::make_shared<Animation>(
            animation.duration,
            animation.ticksPerSecond,
            animation.name,
            std::move(animation.keyframes));

    s_data.animations.insert(std::make_pair(animation.name, outAnimation));
}

void ResourceManager::addTexture(const paca::fileformats::Texture &texture)
{
    Texture::Format format;
    switch (texture.channels)
    {
        case 1: format = Texture::Format::G8; break;
        case 2: format = Texture::Format::GA8; break;
        case 3: format = Texture::Format::RGB8; break;
        case 4: format = Texture::Format::RGBA8; break;
    }

    if (!texture.isCubeMap)
    {
        std::shared_ptr<Texture> outTexture
            = std::make_shared<Texture>(
                reinterpret_cast<const uint8_t*>(texture.pixelData.data()),
                texture.width,
                texture.height,
                format);
        s_data.textures.insert(std::make_pair(texture.name, outTexture));
    }
    else
    {
        std::array<const unsigned char*, 6> facesData;
        for (unsigned int i = 0; i < facesData.size(); i++)
        {
            facesData[i] = reinterpret_cast<const uint8_t*>(
                texture.pixelData.data()
                + texture.width * texture.height * texture.channels * i);
        }
        std::shared_ptr<Texture> outTexture
            = std::make_shared<Texture>(facesData, texture.width, texture.height, format);
        s_data.cubeMaps.insert(std::make_pair(texture.name, outTexture));
    }
}

std::shared_ptr<Texture> ResourceManager::getTexture(const std::string &name)
{
    auto iter = s_data.textures.find(name);
    ASSERT_MSG(iter != s_data.textures.end(), "Texture: {} wasn't added yet.", name);

    return iter->second;
}

std::shared_ptr<Texture> ResourceManager::getCubeMap(const std::string &name)
{
    auto iter = s_data.cubeMaps.find(name);
    ASSERT_MSG(iter != s_data.cubeMaps.end(), "Cubemap: {} wasn't added yet.", name);

    return iter->second;
}

std::shared_ptr<Model> ResourceManager::getModel(const std::string &name)
{
    auto iter = s_data.models.find(name);
    ASSERT_MSG(iter != s_data.models.end(), "Model: {} wasn't added yet.", name);

    std::shared_ptr<Model> model = iter->second;
    ASSERT_MSG(model, "Model: {} was deleted", name);
    return model;
}

std::shared_ptr<Material> ResourceManager::getMaterial(const std::string &name)
{
    auto iter = s_data.materials.find(name);
    ASSERT_MSG(iter != s_data.materials.end(), "Material: {} wasn't added yet.", name);

    std::shared_ptr<Material> material = iter->second;
    ASSERT_MSG(material, "Material is null");
    return material;
}

std::shared_ptr<Animation> ResourceManager::getAnimation(const std::string &name)
{
    auto iter = s_data.animations.find(name);
    ASSERT_MSG(iter != s_data.animations.end(), "Animation: {} wasn't added yet.", name);

    std::shared_ptr<Animation> animation = iter->second;
    ASSERT_MSG(animation, "Animation is null");
    return animation;
}

