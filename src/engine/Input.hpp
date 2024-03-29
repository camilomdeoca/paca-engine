#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#include <functional>
#include <list>

namespace Key {
    enum Keycode {
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

struct ButtonEvent {
    Button::ButtonCode button;
};

struct ButtonPressEvent : public ButtonEvent {};
struct ButtonReleaseEvent : public ButtonEvent {};

struct KeyEvent {
    Key::Keycode key;
};

struct KeyPressEvent : public KeyEvent {};
struct KeyReleaseEvent : public KeyEvent {};

struct MouseMotionEvent {
    int x, y;
    int xRel, yRel;
};

struct MouseWheelEvent {
    int amount;
};

// TODO: Do something better than this :)

typedef std::function<void(ButtonPressEvent&)> ButtonPressCallback;
typedef std::function<void(ButtonReleaseEvent&)> ButtonReleaseCallback;
typedef std::function<void(KeyPressEvent&)> KeyPressCallback;
typedef std::function<void(KeyReleaseEvent&)> KeyReleaseCallback;
typedef std::function<void(MouseMotionEvent&)> MouseMotionCallback;
typedef std::function<void(MouseWheelEvent&)> MouseWheelCallback;

class Input {
public:
    // Should be called after initializing SDL
    static void init();

    static void processInput();

    static void restrainMouseToWindow(bool enabled);

    static bool isKeyPressed(Key::Keycode key);
    static bool isMouseButtonPressed(Button::ButtonCode button);

    static std::list<ButtonPressCallback>::const_iterator addMouseButtonPressCallback(ButtonPressCallback callback, Button::ButtonCode button);
    static std::list<ButtonReleaseCallback>::const_iterator addMouseButtonReleaseCallback(ButtonReleaseCallback callback, Button::ButtonCode button);
    static std::list<KeyPressCallback>::const_iterator addKeyPressCallback(KeyPressCallback callback, Key::Keycode key);
    static std::list<KeyReleaseCallback>::const_iterator addKeyReleaseCallback(KeyReleaseCallback callback, Key::Keycode key);
    static std::list<MouseMotionCallback>::const_iterator addMouseMotionCallback(MouseMotionCallback callback);
    static std::list<MouseWheelCallback>::const_iterator addMouseWheelUpCallback(MouseWheelCallback callback);
    static std::list<MouseWheelCallback>::const_iterator addMouseWheelDownCallback(MouseWheelCallback callback);

    static void removeMouseButtonPressCallback(std::list<ButtonPressCallback>::const_iterator referenceToCallback, Button::ButtonCode button);
    static void removeMouseButtonReleaseCallback(std::list<ButtonReleaseCallback>::const_iterator referenceToCallback, Button::ButtonCode button);
    static void removeKeyPressCallback(std::list<KeyPressCallback>::const_iterator referenceToCallback, Key::Keycode key);
    static void removeKeyReleaseCallback(std::list<KeyReleaseCallback>::const_iterator referenceToCallback, Key::Keycode key);
    static void removeMouseMotionCallback(std::list<MouseMotionCallback>::const_iterator referenceToCallback);
    static void removeMouseWheelUpCallback(std::list<MouseWheelCallback>::const_iterator referenceToCallback);
    static void removeMouseWheelDownCallback(std::list<MouseWheelCallback>::const_iterator referenceToCallback);
};
