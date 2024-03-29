#include "ResourceManager.hpp"

#include "engine/Material.hpp"
#include "engine/Model.hpp"
#include "opengl/Texture.hpp"

#include <cstdio>
#include <optional>
#include <pacaread/pacaread.hpp>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

static struct {
    std::unordered_map<std::string, std::weak_ptr<Texture>> textures;

    // TODO: Change to weak_ptr so the materials and models get deleted when not used.
    // TODO: Add an index file of in wich files are what materials so when the materials are loaded
    // when wanted.
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::unordered_map<std::string, std::shared_ptr<Model>> models; 
} s_data;

std::shared_ptr<Model> ResourceManager::addModel(const std::string &path)
{
    std::optional<paca_read::model> pacaModel = paca_read::read_model(path);
    std::vector<std::shared_ptr<Mesh>> meshes;

    for (const paca_read::mesh &pacaMesh : pacaModel->meshes)
    {
        meshes.emplace_back(std::make_shared<Mesh>(pacaMesh.vertices, pacaMesh.indices, getMaterial(pacaMesh.material_name)));
    }

    std::shared_ptr<Model> model = std::make_shared<Model>(meshes);
    std::weak_ptr<Model> toInsertPtr = model;
    return s_data.models.insert(std::make_pair(pacaModel.value().name, toInsertPtr)).first->second;
}

MaterialTextureType::Type pacaTextureTypeToMaterialTextureType(paca_read::texture_type::type type)
{
    switch (type) {
        case paca_read::texture_type::diffuse:  return MaterialTextureType::diffuse;
        case paca_read::texture_type::specular: return MaterialTextureType::specular;
        case paca_read::texture_type::normal:   return MaterialTextureType::normal;
        case paca_read::texture_type::depth:    return MaterialTextureType::height;
        default: break;
    }

    exit(1);
}

std::shared_ptr<Material> ResourceManager::addMaterial(const std::string &path)
{
    std::optional<paca_read::material> pacaMaterial = paca_read::read_material(path);
    MaterialSpecification materialSpec;
    for (uint32_t i = paca_read::texture_type::none; i < paca_read::texture_type::last; i++)
    {
        for (const paca_read::texture &texture : pacaMaterial->textures[i])
        {
            materialSpec.textureMaps[pacaTextureTypeToMaterialTextureType(paca_read::texture_type::type(i))]
                .emplace_back(getTexture(texture.path));
        }
    }

    std::shared_ptr<Material> material = std::make_shared<Material>(materialSpec);
    return s_data.materials.insert(std::make_pair(pacaMaterial->name, material)).first->second;
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

std::shared_ptr<Model> ResourceManager::getModel(const std::string &name)
{
    std::unordered_map<std::string, std::shared_ptr<Model>>::iterator iter =
        s_data.models.find(name);

    if (iter == s_data.models.end())
    {
        fprintf(stderr, "Model: %s wasn't added yet.\n", name.c_str());
        exit(1);
    }

    std::shared_ptr<Model> model = iter->second;
    if (model)
        return model;

    fprintf(stderr, "Model: %s was deleted", name.c_str());
    exit(1);
}

std::shared_ptr<Material> ResourceManager::getMaterial(const std::string &name)
{
    std::unordered_map<std::string, std::shared_ptr<Material>>::iterator iter =
        s_data.materials.find(name);

    if (iter == s_data.materials.end())
    {
        fprintf(stderr, "Material: %s wasn't added yet.\n", name.c_str());
        exit(1);
    }

    std::shared_ptr<Material> material = iter->second;
    if (material)
        return material;

    exit(1);
}
