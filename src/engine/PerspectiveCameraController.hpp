#pragma once

#include "engine/Input.hpp"
#include "engine/PerspectiveCamera.hpp"

class PerspectiveCameraController {
public:
    PerspectiveCameraController(float aspect, float fov);
    ~PerspectiveCameraController();

    void onUpdate(float ms);

    const PerspectiveCamera &getCamera() { return m_camera; }

    void onMouseScroll(MouseWheelEvent &event);
    void onMouseMotion(MouseMotionEvent &event);

private:
    PerspectiveCamera m_camera;
};

