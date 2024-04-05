#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#include <functional>

namespace Key {
    enum KeyCode {
        q      = SDL_SCANCODE_Q,
        w      = SDL_SCANCODE_W,
        e      = SDL_SCANCODE_E,
        a      = SDL_SCANCODE_A,
        s      = SDL_SCANCODE_S,
        d      = SDL_SCANCODE_D,
        f      = SDL_SCANCODE_F,
        right  = SDL_SCANCODE_RIGHT,
        left   = SDL_SCANCODE_LEFT,
        down   = SDL_SCANCODE_DOWN,
        up     = SDL_SCANCODE_UP,
        lshift = SDL_SCANCODE_LSHIFT,
        space  = SDL_SCANCODE_SPACE,
        esc    = SDL_SCANCODE_ESCAPE
    };
}

namespace Button {
    enum ButtonCode {
        left = SDL_BUTTON_LEFT,
        middle = SDL_BUTTON_MIDDLE,
        right = SDL_BUTTON_RIGHT
    };
}

enum class EventType {
    buttonDown, buttonUp,
    keyDown, keyUp,
    mouseMotion,
    mouseWheelDown, mouseWheelUp,
    windowResize,
    exit,
    last
};

class Event {
public:
    virtual EventType getType() const = 0;
};

class ButtonEvent : public Event {
public:
    ButtonEvent(Button::ButtonCode button) : m_button(button) {}
    Button::ButtonCode getButton() const { return m_button; }
private:
    Button::ButtonCode m_button;
};

class ButtonPressEvent : public ButtonEvent {
public:
    ButtonPressEvent(Button::ButtonCode button) : ButtonEvent(button) {}
    EventType getType() const override { return EventType::buttonDown; }
};

class ButtonReleaseEvent : public ButtonEvent {
public:
    ButtonReleaseEvent(Button::ButtonCode button) : ButtonEvent(button) {}
    EventType getType() const override { return EventType::buttonUp; }
};

class KeyEvent : public Event {
public:
    KeyEvent(Key::KeyCode key) : m_key(key) {}
    Key::KeyCode getKey() const { return m_key; }
private:
    Key::KeyCode m_key;
};

class KeyPressEvent : public KeyEvent {
public:
    KeyPressEvent(Key::KeyCode key) : KeyEvent(key) {}
    EventType getType() const override { return EventType::keyDown; }
};

class KeyReleaseEvent : public KeyEvent {
public:
    KeyReleaseEvent(Key::KeyCode key) : KeyEvent(key) {}
    EventType getType() const override { return EventType::keyUp; }
};

class MouseMotionEvent : public Event {
public:
    MouseMotionEvent(int x, int y, int xRelative, int yRelative)
        : m_x(x), m_y(y), m_xRel(xRelative), m_yRel(yRelative) {}
    EventType getType() const override { return EventType::mouseMotion; }
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getRelativeX() const { return m_xRel; }
    int getRelativeY() const { return m_yRel; }
private:
    int m_x, m_y;
    int m_xRel, m_yRel;
};

class MouseWheelEvent : public Event {
public:
    MouseWheelEvent(int ammount) : m_amount(ammount) {}
    int getAmmount() const { return m_amount; }
private:
    int m_amount;
};

class MouseWheelDownEvent : public MouseWheelEvent {
public:
    MouseWheelDownEvent(int ammount) : MouseWheelEvent(ammount) {}
    EventType getType() const override { return EventType::mouseWheelDown; }
};

class MouseWheelUpEvent : public MouseWheelEvent {
public:
    MouseWheelUpEvent(int ammount) : MouseWheelEvent(ammount) {}
    EventType getType() const override { return EventType::mouseWheelUp; }
};

class ResizeEvent : public Event {
public:
    ResizeEvent(unsigned int width, unsigned int height) : m_w(width), m_h(height) {}
    EventType getType() const override { return EventType::windowResize; }
    unsigned int getWidth() const { return m_w; }
    unsigned int getHeight() const { return m_h; }
private:
    unsigned int m_w, m_h;
};

class ExitEvent : public Event {
public:
    EventType getType() const override { return EventType::exit; }
};

namespace EventMask {
    enum Mask : uint32_t {
        buttonDown     = 1 << 0,
        buttonUp       = 1 << 1,
        keyDown        = 1 << 2,
        keyUp          = 1 << 3,
        mouseMotion    = 1 << 4,
        mouseWheelDown = 1 << 5,
        mouseWheelUp   = 1 << 6,
        windowResize   = 1 << 7,
        exit           = 1 << 8,
    };
}

class EventReceiver {
public:
    EventReceiver();
    ~EventReceiver();

    void setEventsMask(uint32_t eventsMask);
    void setEventHandler(std::function<void(const Event&)> handler) { m_eventHandler = handler; }
    void onEvent(const Event &event) { m_eventHandler(event); }
private:
    std::function<void(const Event&)> m_eventHandler;
    uint32_t m_eventsMask = 0;
};

class Input {
public:
    // Should be called after initializing SDL
    static void init();

    static void processInput();

    static void restrainMouseToWindow(bool enabled);

    static bool isKeyPressed(Key::KeyCode key);
    static bool isMouseButtonPressed(Button::ButtonCode button);
};
