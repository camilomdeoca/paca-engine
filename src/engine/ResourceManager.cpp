#include "ResourceManager.hpp"

#include "engine/AnimatedMesh.hpp"
#include "engine/Assert.hpp"
#include "engine/Material.hpp"
#include "engine/Mesh.hpp"
#include "engine/Model.hpp"
#include "engine/StaticMesh.hpp"
#include "opengl/Texture.hpp"

#include <functional>
#include <optional>
#include <pacaread/pacaread.hpp>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

struct StringAndViewHash : public std::hash<std::string_view> { using is_transparent = void; };

static struct {
    std::unordered_map<std::string, std::weak_ptr<Texture>, StringAndViewHash, std::equal_to<>> textures;
    std::unordered_map<std::string, std::weak_ptr<Texture>, StringAndViewHash, std::equal_to<>> cubeMaps;

    // TODO: Change to weak_ptr so the materials and models get deleted when not used.
    // TODO: Add an index file of in wich files are what materials so when the materials are loaded
    // when wanted.
    std::unordered_map<std::string, std::shared_ptr<Material>, StringAndViewHash, std::equal_to<>> materials;
    std::unordered_map<std::string, std::shared_ptr<Model>, StringAndViewHash, std::equal_to<>> models; 
    std::unordered_map<std::string, std::shared_ptr<Animation>, StringAndViewHash, std::equal_to<>> animations; 
} s_data;

std::shared_ptr<Model> ResourceManager::addModel(const std::string &path)
{
    std::optional<paca_format::Model> pacaModel = paca_format::readModel(path);
    std::vector<std::shared_ptr<Mesh>> meshes;

#ifdef DEBUG
    size_t vertexCount = 0;
#endif // DEBUG

    for (paca_format::Mesh &pacaMesh : pacaModel->meshes)
    {
#ifdef DEBUG
        vertexCount += pacaMesh.vertices.size() / paca_format::vertexTypeToSize(pacaMesh.vertexType);
#endif // DEBUG
        
        std::vector<std::shared_ptr<Animation>> animations;
        for (const std::string &animName : pacaMesh.animations)
            animations.emplace_back(getAnimation(animName));
        // if mesh is animated
        if (pacaMesh.vertexType == paca_format::VertexType::float3pos_float3norm_float3tang_float2texture_int4boneIds_float4boneWeights)
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
    INFO("Model {} has {} vertices", path, vertexCount);

    std::shared_ptr<Model> model = std::make_shared<Model>(meshes);
    std::weak_ptr<Model> toInsertPtr = model;
    return s_data.models.insert(std::make_pair(pacaModel.value().name, toInsertPtr)).first->second;
}

MaterialTextureType::Type pacaTextureTypeToMaterialTextureType(paca_format::TextureType::Type type)
{
    switch (type) {
        case paca_format::TextureType::diffuse:  return MaterialTextureType::diffuse;
        case paca_format::TextureType::specular: return MaterialTextureType::specular;
        case paca_format::TextureType::normal:   return MaterialTextureType::normal;
        case paca_format::TextureType::depth:    return MaterialTextureType::height;
        default: break;
    }

    ASSERT_MSG(false, "Invalid paca texture type!");
}

std::shared_ptr<Material> ResourceManager::addMaterial(const std::string &path)
{
    std::optional<paca_format::Material> pacaMaterial = paca_format::readMaterial(path);
    MaterialSpecification materialSpec;
    for (uint32_t i = paca_format::TextureType::none; i < paca_format::TextureType::last; i++)
    {
        for (const paca_format::Texture &texture : pacaMaterial->textures[i])
        {
            materialSpec.textureMaps[pacaTextureTypeToMaterialTextureType(paca_format::TextureType::Type(i))]
                .emplace_back(getTexture(texture.path));
        }
    }

    std::shared_ptr<Material> material = std::make_shared<Material>(materialSpec);
    return s_data.materials.insert(std::make_pair(pacaMaterial->name, material)).first->second;
}

std::shared_ptr<Animation> ResourceManager::addAnimation(const std::string &path)
{
    std::optional<paca_format::Animation> pacaAnimation = paca_format::readAnimation(path);
    ASSERT(pacaAnimation.has_value());
    std::shared_ptr<Animation> animation = std::make_shared<Animation>(
            pacaAnimation->duration,
            pacaAnimation->ticksPerSecond,
            std::move(pacaAnimation->keyframes));

    return s_data.animations.insert(std::make_pair(pacaAnimation->name, animation)).first->second;
}

std::shared_ptr<Texture> ResourceManager::getTexture(const std::string &path)
{
    const std::string texturesFolder = "assets/textures/";
    std::unordered_map<std::string, std::weak_ptr<Texture>>::iterator iter =
        s_data.textures.find(path);

    if (iter == s_data.textures.end())
    {
        std::shared_ptr<Texture> texture = std::make_shared<Texture>(texturesFolder + path);
        std::weak_ptr<Texture> toInsertPtr = texture;
        s_data.textures.insert(std::make_pair(path, toInsertPtr));
        return texture;
    }

    std::shared_ptr<Texture> texture = iter->second.lock();
    if (texture)
        return texture;

    s_data.textures.erase(path);
    texture = std::make_shared<Texture>(texturesFolder + path);
    std::weak_ptr<Texture> toInsertPtr = texture;
    s_data.textures.insert(std::make_pair(path, toInsertPtr));
    return texture;
}

std::shared_ptr<Texture> ResourceManager::getCubeMap(const std::string &folder)
{
    const std::string cubeMapsFolder = "assets/textures/cubemaps/";
    const std::array<std::string, 6> facesNames = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    };
    std::unordered_map<std::string, std::weak_ptr<Texture>>::iterator iter =
        s_data.cubeMaps.find(folder);

    if (iter == s_data.cubeMaps.end())
    {
        std::array<std::string, 6> filePaths;
        for (unsigned int i = 0; i < facesNames.size(); i++)
            filePaths[i] = cubeMapsFolder + folder + facesNames[i];
        std::shared_ptr<Texture> cubeMap = std::make_shared<Texture>(filePaths);
        std::weak_ptr<Texture> toInsertPtr = cubeMap;
        s_data.cubeMaps.insert(std::make_pair(folder, toInsertPtr));
        return cubeMap;
    }

    std::shared_ptr<Texture> cubeMap = iter->second.lock();
    if (cubeMap)
        return cubeMap;

    s_data.cubeMaps.erase(folder);
    std::array<std::string, 6> filePaths;
    for (unsigned int i = 0; i < facesNames.size(); i++)
        filePaths[i] = cubeMapsFolder + folder + facesNames[i];
    cubeMap = std::make_shared<Texture>(filePaths);
    std::weak_ptr<Texture> toInsertPtr = cubeMap;
    s_data.cubeMaps.insert(std::make_pair(folder, toInsertPtr));
    return cubeMap;
}

std::shared_ptr<Model> ResourceManager::getModel(const std::string &name)
{
    std::unordered_map<std::string, std::shared_ptr<Model>>::iterator iter =
        s_data.models.find(name);
    ASSERT_MSG(iter != s_data.models.end(), "Model: {} wasn't added yet.", name);

    std::shared_ptr<Model> model = iter->second;
    ASSERT_MSG(model, "Model: {} was deleted", name);
    return model;
}

std::shared_ptr<Material> ResourceManager::getMaterial(const std::string &name)
{
    std::unordered_map<std::string, std::shared_ptr<Material>>::iterator iter =
        s_data.materials.find(name);
    ASSERT_MSG(iter != s_data.materials.end(), "Material: {} wasn't added yet.", name);

    std::shared_ptr<Material> material = iter->second;
    ASSERT_MSG(material, "Material is null");
    return material;
}

std::shared_ptr<Animation> ResourceManager::getAnimation(const std::string &name)
{
    std::unordered_map<std::string, std::shared_ptr<Animation>>::iterator iter =
        s_data.animations.find(name);
    ASSERT_MSG(iter != s_data.animations.end(), "Animation: {} wasn't added yet.", name);

    std::shared_ptr<Animation> animation = iter->second;
    ASSERT_MSG(animation, "Animation is null");
    return animation;
}

