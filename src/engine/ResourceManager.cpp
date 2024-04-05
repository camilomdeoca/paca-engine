#include "ResourceManager.hpp"

#include "engine/Material.hpp"
#include "engine/Model.hpp"
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

    // TODO: Change to weak_ptr so the materials and models get deleted when not used.
    // TODO: Add an index file of in wich files are what materials so when the materials are loaded
    // when wanted.
    std::unordered_map<std::string, std::shared_ptr<Material>, StringAndViewHash, std::equal_to<>> materials;
    std::unordered_map<std::string, std::shared_ptr<Model>, StringAndViewHash, std::equal_to<>> models; 
} s_data;

std::shared_ptr<Model> ResourceManager::addModel(std::string_view &path)
{
    std::optional<paca_format::Model> pacaModel = paca_format::readModel(path);
    std::vector<std::shared_ptr<Mesh>> meshes;

    for (const paca_format::Mesh &pacaMesh : pacaModel->meshes)
    {
        meshes.emplace_back(std::make_shared<Mesh>(pacaMesh.vertices, pacaMesh.indices, getMaterial(pacaMesh.materialName)));
    }

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

    exit(1);
}

std::shared_ptr<Material> ResourceManager::addMaterial(std::string_view path)
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

std::shared_ptr<Texture> ResourceManager::getTexture(std::string_view path)
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

std::shared_ptr<Model> ResourceManager::getModel(std::string_view name)
{
    std::unordered_map<std::string, std::shared_ptr<Model>>::iterator iter =
        s_data.models.find(name);

    if (iter == s_data.models.end())
    {
        ERROR("Model: {} wasn't added yet.", name);
        ASSERT(false);
    }

    std::shared_ptr<Model> model = iter->second;
    if (model)
        return model;

    ERROR("Model: {} was deleted", name);
    ASSERT(false);
}

std::shared_ptr<Material> ResourceManager::getMaterial(std::string_view name)
{
    std::unordered_map<std::string, std::shared_ptr<Material>>::iterator iter =
        s_data.materials.find(name);

    if (iter == s_data.materials.end())
    {
        ERROR("Material: {} wasn't added yet.", name);
        ASSERT(false);
    }

    std::shared_ptr<Material> material = iter->second;
    if (material)
        return material;

    ASSERT_MSG(false, "Material is null");
}
