#pragma once

#include "engine/Model.hpp"
#include <glm/glm.hpp>
#include <memory>

class Light {
public:
    Light(const glm::vec3 &color, float intensity) : m_color(color), m_intensity(intensity) {}

    const glm::vec3 &getColor() const { return m_color; }
    const float &getIntensity() const { return m_intensity; }

    void setColor(const glm::vec3 &color) { m_color = color; }
    void setIntensity(float intensity) { m_intensity = intensity; }

private:
    glm::vec3 m_color;
    float m_intensity;
};

class PointLight : public Light {
public:
    PointLight(const glm::vec3 &position, const glm::vec3 &color, float intensity, float attenuation, std::shared_ptr<Model> model = nullptr)
        : Light(color, intensity), m_position(position), m_attenuation(attenuation), m_model(model) {}

    const glm::vec3 &getPosition() const { return m_position; }
    const float &getAttenuation() const { return m_attenuation; }
    const std::shared_ptr<Model> getModel() const { return m_model; }

    void setPosition(const glm::vec3 &position) { m_position = position; }
    void setAttenuation(float attenuation) { m_attenuation = attenuation; }

private:
    glm::vec3 m_position = {0.0f, 0.0f, 0.0f};
    float m_attenuation;
    std::shared_ptr<Model> m_model;
};

class DirectionalLight : public Light {
public:
    DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color, float intensity)
        : Light(color, intensity), m_direction(direction) {}

    const glm::vec3 &getDirection() const { return m_direction; }

    void setDirection(const glm::vec3 &direction) { m_direction = direction; }

private:
    glm::vec3 m_direction;
};

