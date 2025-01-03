#pragma once

#include <serializers/Reflection.hpp>

#include <glm/glm.hpp>

namespace engine::components {

struct DirectionalLight
{
    NAME("DirectionalLight")
    FIELDS(color, intensity)
    FIELD_NAMES("color", "intensity")
    glm::vec3 color;
    float intensity;
};

}
