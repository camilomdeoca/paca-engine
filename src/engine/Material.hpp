#pragma once

#include "opengl/Texture.hpp"

#include <memory>
#include <vector>
#include <array>

namespace MaterialTextureType {
    enum Type {
        diffuse,
        specular,
        normal,
        height,
    
        last
    };
}

struct MaterialSpecification {
    // textureMaps[MaterialTextureType::Type] is the vector that holds the textures of the texture type of the index
    std::array<std::vector<std::shared_ptr<Texture>>, static_cast<size_t>(MaterialTextureType::last)> textureMaps;
};

class Material {
public:
    Material(MaterialSpecification maps);
    ~Material();

    const std::vector<std::shared_ptr<Texture>> &getTextures(MaterialTextureType::Type type);
    static std::string MaterialTextureTypeToUniformName(MaterialTextureType::Type type);

private:
    std::array<std::vector<std::shared_ptr<Texture>>, MaterialTextureType::last> m_maps;
};
