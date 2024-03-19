#include "Light.hpp"

Light::Light()
{}

Light::Light(const glm::vec3 &position, const glm::vec3 &color, float intensity, float attenuation)
    : m_position(position), m_color(color), m_intensity(intensity), m_attenuation(attenuation)
{}

Light::~Light()
{}

