#pragma once

#include "engine/Mesh.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Model {
public:
    Model(const std::vector<std::shared_ptr<Mesh>> &meshes);
    ~Model();

    void setPosition(const glm::vec3 &position) { m_position = position; }
    void setRotation(const glm::vec3 &rotation) { m_rotation = rotation; }
    void setScale(const glm::vec3 &scale) { m_scale = scale; }

    const std::vector<std::shared_ptr<Mesh>> &getMeshes() const { return m_meshes; }
    const glm::vec3 &getPosition() const { return m_position; }
    const glm::vec3 &getRotation() const { return m_rotation; }
    const glm::vec3 &getScale() const { return m_scale; }

private:
    std::vector<std::shared_ptr<Mesh>> m_meshes;
    glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
    glm::vec3 m_rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 m_scale = {1.0f, 1.0f, 1.0f};
};
