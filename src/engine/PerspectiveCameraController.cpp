#include "PerspectiveCameraController.hpp"

#include "engine/Input.hpp"

PerspectiveCameraController::PerspectiveCameraController(float aspect, float fov)
    : m_camera(aspect, fov)
{}

PerspectiveCameraController::~PerspectiveCameraController()
{
    if (m_haveControl) {
        Input::removeMouseWheelDownCallback(m_mouseWheelDownCallbackReference);
        Input::removeMouseWheelUpCallback(m_mouseWheelUpCallbackReference);
        Input::removeMouseMotionCallback(m_mouseMotionCallbackReference);
    }
}

void PerspectiveCameraController::onUpdate(float ms)
{
    if (m_haveControl) {
        constexpr float cameraSpeed = 0.005f; // units per ms

        glm::vec3 position = getCamera().getPosition();
        glm::vec3 front = getCamera().getDirection();
        front.y = 0;
        front = glm::normalize(front);
        glm::vec3 right = glm::normalize(glm::cross(front, getCamera().getUp()));

        if (Input::isKeyPressed(Key::a)) {
            position -= right * cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::d)) {
            position += right * cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::w)) {
            position += front * cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::s)) {
            position -= front * cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::space)) {
            position.y += cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::lshift)) {
            position.y -= cameraSpeed * ms;
        }

        m_camera.setPosition(position);
    }
}

void PerspectiveCameraController::onMouseScroll(MouseWheelEvent &event)
{
    float fov = m_camera.getFov();
    fov *= 1.0f - event.amount * 0.1f;
    if (fov > 120.0f) fov = 120.0f;
    // fov -= event.amount * 0.25f;
    m_camera.setFov(fov);
}

void PerspectiveCameraController::onMouseMotion(MouseMotionEvent &event)
{
    const float sensitivity = 0.1f;
    glm::vec3 rotation = m_camera.getRotation();
    float fov = m_camera.getFov();

    rotation.x -= event.yRel * sensitivity * (fov/90.0f);
    rotation.y += event.xRel * sensitivity * (fov/90.0f);

    rotation.x = std::clamp(rotation.x, -89.0f, 89.0f);

    m_camera.setRotation(rotation);
}

void PerspectiveCameraController::pauseControl()
{
    if (m_haveControl) {
        Input::removeMouseWheelDownCallback(m_mouseWheelDownCallbackReference);
        Input::removeMouseWheelUpCallback(m_mouseWheelUpCallbackReference);
        Input::removeMouseMotionCallback(m_mouseMotionCallbackReference);
        m_haveControl = false;
    }
}

void PerspectiveCameraController::resumeControl()
{
    if (!m_haveControl) {
        MouseWheelCallback callback = [this](MouseWheelEvent &event) { onMouseScroll(event); };
        m_mouseWheelDownCallbackReference = Input::addMouseWheelDownCallback(callback); // TODO: remove callback on destructor
        m_mouseWheelUpCallbackReference = Input::addMouseWheelUpCallback(callback); // TODO: remove callback on destructor
        m_mouseMotionCallbackReference = Input::addMouseMotionCallback([this](MouseMotionEvent &event) { onMouseMotion(event); });
        m_haveControl = true;
    }
}

