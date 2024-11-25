#pragma once

#include <glm/glm.hpp>

namespace engine::components {

struct DirectionalLight
{
    glm::vec3 color;
    float intensity;
};

}
