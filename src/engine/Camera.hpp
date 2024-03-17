#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    virtual const glm::vec3 &getPosition() const = 0;
    //virtual void setPosition(const glm::vec3 &position) = 0;

    // Pitch, yaw, roll
    virtual const glm::vec3 &getRotation() const = 0;
    //virtual void setRotation(glm::vec3 rotation) = 0;
    virtual const glm::vec3 &getDirection() const = 0;
    virtual const glm::vec3 &getUp() const = 0;


    virtual const glm::mat4 &getViewProjectionMatrix() const = 0;
    virtual const glm::mat4 &getProjectionMatrix() const = 0;
    virtual const glm::mat4 &getViewMatrix() const = 0;
};
