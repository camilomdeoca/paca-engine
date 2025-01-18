#pragma once

#include "engine/IdTypes.hpp"

#include <vector>
#include <array>

namespace MaterialTextureType {
    enum Type {
        diffuse,
        specular,
        normal,
        height,

        last,
    };
}

struct MaterialSpecification {
    // textures[MaterialTextureType::Type] is the vector that holds the textures of the texture type of the index
    std::array<std::vector<TextureId>, static_cast<size_t>(MaterialTextureType::last)> textures;
};

class Material {
public:
    Material(MaterialSpecification specification);
    ~Material();

    const std::vector<TextureId> &getTextureIds(MaterialTextureType::Type type) const;
    static std::string TextureTypeToUniformName(MaterialTextureType::Type type);

private:
    std::array<std::vector<TextureId>, MaterialTextureType::last> m_textures;
};
