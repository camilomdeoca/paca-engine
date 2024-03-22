#pragma once

#include "engine/Model.hpp"
#include <glm/glm.hpp>
#include <memory>

class Light {
public:
    Light();
    Light(const glm::vec3 &position, const glm::vec3 &color, float intensity, float attenuation, std::shared_ptr<Model> model = nullptr);
    ~Light();

    const glm::vec3 &getPosition() const { return m_position; }
    const glm::vec3 &getColor() const { return m_color; }
    const float &getIntensity() const { return m_intensity; }
    const float &getAttenuation() const { return m_attenuation; }
    const std::shared_ptr<Model> getModel() const { return m_model; }

    void setPosition(const glm::vec3 &position) { m_position = position; }
    void setColor(const glm::vec3 &color) { m_color = color; }
    void setIntensity(float intensity) { m_intensity = intensity; }
    void setAttenuation(float attenuation) { m_attenuation = attenuation; }

private:
    glm::vec3 m_position;
    glm::vec3 m_color;
    float m_intensity;
    float m_attenuation;
    std::shared_ptr<Model> m_model;
};
