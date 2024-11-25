#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine::components {

struct Transform
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};

    operator const glm::mat4() const
    {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::scale(transform, scale);
        glm::quat rot(glm::radians(rotation));
        transform = transform * glm::mat4_cast(rot);
        return transform;
    }

    glm::mat3 getRotationMat3() const
    {
        return glm::mat3_cast(glm::quat(glm::radians(rotation)));
    }
};

}
