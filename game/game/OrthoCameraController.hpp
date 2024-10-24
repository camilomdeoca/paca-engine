#pragma once

#include "game/Input.hpp"
#include "engine/OrthoCamera.hpp"

class OrthoCameraController {
public:
    OrthoCameraController(float aspectRatio);
    ~OrthoCameraController();

    void onUpdate(float ms);

    //void onKeyPress(KeyPressEvent &event);
    OrthoCamera &getCamera() { return m_camera; }

    void onMouseScroll(const MouseWheelEvent &event);

private:
    float m_aspectRatio;
    float m_zoom = 1.0f;
    OrthoCamera m_camera;
    EventReceiver m_eventReceiver;
};
