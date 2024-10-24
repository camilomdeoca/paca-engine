#pragma once

#include "game/Action.hpp"
#include "game/Input.hpp"
#include "engine/PerspectiveCamera.hpp"

class PerspectiveCameraController {
public:
    PerspectiveCameraController(float aspect, float fov);
    ~PerspectiveCameraController();

    void onUpdate(float ms);

    const PerspectiveCamera &getCamera() { return m_camera; }
    void setAspect(float aspect) { m_camera.setAspect(aspect); }

    void onMouseMotion(const MouseMotionEvent &event);

private:
    struct DirectionMask {
        enum Mask {
            forward  = 1 << 0,
            backward = 1 << 1,
            left     = 1 << 2,
            right    = 1 << 3,
            up       = 1 << 4,
            down     = 1 << 5
        };
    };

    PerspectiveCamera m_camera;
    EventReceiver m_eventReceiver;
    std::array<Action, 8> m_actions;
    bool m_haveControl = true;
    unsigned int m_moving = 0; // bitmask for DirectionMask
};

