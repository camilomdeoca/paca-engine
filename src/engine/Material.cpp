#include "Material.hpp"
#include <cstdio>

Material::Material(MaterialSpecification maps)
    : m_maps(maps.textureMaps)
{}

Material::~Material()
{}

const std::vector<std::shared_ptr<Texture>> &Material::getTextures(MaterialTextureType::Type type)
{
    return m_maps[static_cast<size_t>(type)];
}

std::string MaterialTextureTypeToUniformName(MaterialTextureType::Type type)
{
    switch (type) {
        case MaterialTextureType::diffuse: return "u_diffuse";
        case MaterialTextureType::specular: return "u_specular";
        case MaterialTextureType::normal:
        case MaterialTextureType::height:
        default: break;
    }

    fprintf(stderr, "Invalid Material Texture Type!\n");
    exit(1);
}

