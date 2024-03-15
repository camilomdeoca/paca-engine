#include "Input.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_scancode.h>
#include <array>
#include <functional>

static const Uint8 *s_keyboardState = nullptr;

constexpr int NUM_OF_MOUSE_BUTTONS = 5;

static std::array<std::vector<std::function<void(ButtonPressEvent&)>>, NUM_OF_MOUSE_BUTTONS> s_buttonPressCallbacks;
static std::array<std::vector<std::function<void(ButtonReleaseEvent&)>>, NUM_OF_MOUSE_BUTTONS> s_buttonReleaseCallbacks;
static std::array<std::vector<std::function<void(KeyPressEvent&)>>, SDL_NUM_SCANCODES> s_keyPressCallbacks;
static std::array<std::vector<std::function<void(KeyReleaseEvent&)>>, SDL_NUM_SCANCODES> s_keyReleaseCallbacks;
static std::vector<std::function<void(MouseMotionEvent&)>> s_mouseMotionCallbacks;
static std::vector<std::function<void(MouseWheelEvent&)>> s_scrollWheelUpCallbacks;
static std::vector<std::function<void(MouseWheelEvent&)>> s_scrollWheelDownCallbacks;

void Input::init()
{
    s_keyboardState = SDL_GetKeyboardState(nullptr);
}

void Input::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_KEYDOWN:
                {
                    KeyPressEvent keyPressEvent = {
                        Key::Keycode(event.key.keysym.scancode)
                    };
                    for (std::function<void(KeyPressEvent&)> callback : s_keyPressCallbacks[keyPressEvent.key])
                        callback(keyPressEvent);
                }
                break;
            case SDL_KEYUP:
                {
                    KeyReleaseEvent keyReleaseEvent = {
                        Key::Keycode(event.key.keysym.scancode)
                    };
                    for (std::function<void(KeyReleaseEvent&)> callback : s_keyReleaseCallbacks[keyReleaseEvent.key])
                        callback(keyReleaseEvent);
                }
                break;
            case SDL_MOUSEMOTION:
                {
                    MouseMotionEvent mouseMotionEvent = {
                        event.motion.x, event.motion.y,
                        event.motion.xrel, event.motion.yrel
                    };
                    for (std::function<void(MouseMotionEvent&)> callback : s_mouseMotionCallbacks)
                        callback(mouseMotionEvent);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                {
                    ButtonPressEvent buttonPressEvent = {
                        Button::ButtonCode(event.button.button)
                    };
                    for (std::function<void(ButtonPressEvent&)> callback : s_buttonPressCallbacks[buttonPressEvent.button])
                        callback(buttonPressEvent);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                {
                    ButtonReleaseEvent buttonReleaseEvent = {
                        Button::ButtonCode(event.button.button)
                    };
                    for (std::function<void(ButtonReleaseEvent&)> callback : s_buttonReleaseCallbacks[buttonReleaseEvent.button])
                        callback(buttonReleaseEvent);
                }
                break;
            case SDL_MOUSEWHEEL:
                {
                    MouseWheelEvent mouseWheelEvent = {
                        event.wheel.y
                    };
                    auto callbacks = mouseWheelEvent.amount > 0 ? s_scrollWheelUpCallbacks : s_scrollWheelDownCallbacks;
                    for (std::function<void(MouseWheelEvent&)> callback : callbacks)
                        callback(mouseWheelEvent);
                }
                break;
        }
    }
}

void Input::restrainMouseToWindow(bool enabled)
{
    SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
}

bool Input::isKeyPressed(Key::Keycode key)
{
    return s_keyboardState[key];
}

bool Input::isMouseButtonPressed(Button::ButtonCode button)
{
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(button);
}

void Input::addMouseButtonPressCallback(std::function<void(ButtonPressEvent&)> callback, Button::ButtonCode button)
{
    s_buttonPressCallbacks[button].push_back(callback);
}

void Input::addMouseButtonReleaseCallback(std::function<void(ButtonReleaseEvent&)> callback, Button::ButtonCode button)
{
    s_buttonReleaseCallbacks[button].push_back(callback);
}


void Input::addKeyPressCallback(std::function<void(KeyPressEvent&)> callback, Key::Keycode key)
{
    s_keyPressCallbacks[key].push_back(callback);
}

void Input::addKeyReleaseCallback(std::function<void(KeyReleaseEvent&)> callback, Key::Keycode key)
{
    s_keyReleaseCallbacks[key].push_back(callback);
}

void Input::addMouseMotionCallback(std::function<void(MouseMotionEvent&)> callback)
{
    s_mouseMotionCallbacks.push_back(callback);
}

void Input::addMouseWheelUpCallback(std::function<void(MouseWheelEvent&)> callback)
{
    s_scrollWheelUpCallbacks.push_back(callback);
}

void Input::addMouseWheelDownCallback(std::function<void(MouseWheelEvent&)> callback)
{
    s_scrollWheelDownCallbacks.push_back(callback);
}

