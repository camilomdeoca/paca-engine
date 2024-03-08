#include "OrthoCameraController.hpp"
#include "Input.hpp"

OrthoCameraController::OrthoCameraController(float aspectRatio)
    : m_aspectRatio(aspectRatio), m_camera(-m_aspectRatio*m_zoom, m_aspectRatio*m_zoom, -m_zoom, m_zoom)
{}

void OrthoCameraController::onUpdate(float ms)
{
    constexpr float cameraSpeed = 0.01f; // units per ms
    constexpr float rotationSpeed = 0.1f; // degrees per ms

    glm::vec3 position = getCamera().getPosition();
    float rotation = getCamera().getRotation();
    
    if (Input::isKeyPressed(Key::a)) {
        position.x -= cos(glm::radians(rotation)) * cameraSpeed * ms;
        position.y -= sin(glm::radians(rotation)) * cameraSpeed * ms;
    }
    if (Input::isKeyPressed(Key::d)) {
        position.x += cos(glm::radians(rotation)) * cameraSpeed * ms;
        position.y += sin(glm::radians(rotation)) * cameraSpeed * ms;
    }
    if (Input::isKeyPressed(Key::w)) {
        position.x += -sin(glm::radians(rotation)) * cameraSpeed * ms;
        position.y += cos(glm::radians(rotation)) * cameraSpeed * ms;
    }
    if (Input::isKeyPressed(Key::s)) {
        position.x -= -sin(glm::radians(rotation)) * cameraSpeed * ms;
        position.y -= cos(glm::radians(rotation)) * cameraSpeed * ms;
    }

    if (Input::isKeyPressed(Key::q)) {
        rotation -= rotationSpeed * ms;
    }
    if (Input::isKeyPressed(Key::e)) {
        rotation += rotationSpeed * ms;
    }
    if (rotation > 180.0f) rotation -= 360.0f;
    if (rotation <= -180.0f) rotation += 360.0f;

    getCamera().setPosition(position);
    getCamera().setRotation(rotation);
}


