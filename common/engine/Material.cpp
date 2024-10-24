#include "engine/Material.hpp"

Material::Material(MaterialSpecification maps)
    : m_maps(maps.textureMaps)
{}

Material::~Material()
{}

const std::vector<std::shared_ptr<Texture>> &Material::getTextures(MaterialTextureType::Type type)
{
    return m_maps[static_cast<size_t>(type)];
}

