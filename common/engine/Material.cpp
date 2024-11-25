#include "engine/Material.hpp"

Material::Material(MaterialSpecification specification)
    : m_textures(specification.textures)
{}

Material::~Material()
{}

const std::vector<TextureId> &Material::getTextureIds(MaterialTextureType::Type type) const
{
    return m_textures[static_cast<size_t>(type)];
}

