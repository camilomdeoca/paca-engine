#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#include <functional>

namespace Key {
    enum Keycode {
        q      = SDL_SCANCODE_Q,
        w      = SDL_SCANCODE_W,
        e      = SDL_SCANCODE_E,
        a      = SDL_SCANCODE_A,
        s      = SDL_SCANCODE_S,
        d      = SDL_SCANCODE_D,
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

class Input {
public:
    // Should be called after initializing SDL
    static void init();

    static void processInput();

    static bool isKeyPressed(Key::Keycode key);
    static bool isMouseButtonPressed(Button::ButtonCode button);

    static void addMouseButtonPressCallback(std::function<void(ButtonPressEvent&)> callback, Button::ButtonCode button);
    static void addMouseButtonReleaseCallback(std::function<void(ButtonReleaseEvent&)> callback, Button::ButtonCode button);
    static void addKeyPressCallback(std::function<void(KeyPressEvent&)> callback, Key::Keycode key);
    static void addKeyReleaseCallback(std::function<void(KeyReleaseEvent&)> callback, Key::Keycode key);
    static void addMouseMotionCallback(std::function<void(MouseMotionEvent&)> callback);
};
