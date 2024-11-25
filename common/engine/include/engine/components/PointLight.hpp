#pragma once

#include <glm/glm.hpp>

namespace engine::components {

struct PointLight
{
    glm::vec3 color;
    float intensity;
    float attenuation;
};

}
