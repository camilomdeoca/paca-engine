#pragma once

#include <serializers/Reflection.hpp>

#include <glm/glm.hpp>

namespace engine::components {

struct PointLight
{
    NAME("PointLight")
    FIELDS(color, intensity, attenuation)
    FIELD_NAMES("color", "intensity", "attenuation")
    glm::vec3 color;
    float intensity;
    float attenuation;
};

}
