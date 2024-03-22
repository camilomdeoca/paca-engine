#include "Input.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_scancode.h>

static const Uint8 *s_keyboardState = nullptr;

constexpr int NUM_OF_MOUSE_BUTTONS = 5;

static std::array<std::list<std::function<void(ButtonPressEvent&)>>, NUM_OF_MOUSE_BUTTONS> s_buttonPressCallbacks;
static std::array<std::list<std::function<void(ButtonReleaseEvent&)>>, NUM_OF_MOUSE_BUTTONS> s_buttonReleaseCallbacks;
static std::array<std::list<std::function<void(KeyPressEvent&)>>, SDL_NUM_SCANCODES> s_keyPressCallbacks;
static std::array<std::list<std::function<void(KeyReleaseEvent&)>>, SDL_NUM_SCANCODES> s_keyReleaseCallbacks;
static std::list<std::function<void(MouseMotionEvent&)>> s_mouseMotionCallbacks;
static std::list<std::function<void(MouseWheelEvent&)>> s_scrollWheelUpCallbacks;
static std::list<std::function<void(MouseWheelEvent&)>> s_scrollWheelDownCallbacks;

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

std::list<ButtonPressCallback>::const_iterator Input::addMouseButtonPressCallback(ButtonPressCallback callback, Button::ButtonCode button)
{
    return s_buttonPressCallbacks[button].insert(s_buttonPressCallbacks[button].end(), callback);
}

std::list<ButtonReleaseCallback>::const_iterator Input::addMouseButtonReleaseCallback(ButtonReleaseCallback callback, Button::ButtonCode button)
{
    return s_buttonReleaseCallbacks[button].insert(s_buttonReleaseCallbacks[button].end(), callback);
}

std::list<KeyPressCallback>::const_iterator Input::addKeyPressCallback(KeyPressCallback callback, Key::Keycode key)
{
    return s_keyPressCallbacks[key].insert(s_keyPressCallbacks[key].end(), callback);
}

std::list<KeyReleaseCallback>::const_iterator Input::addKeyReleaseCallback(KeyReleaseCallback callback, Key::Keycode key)
{
     return s_keyReleaseCallbacks[key].insert(s_keyReleaseCallbacks[key].end(), callback);
}

std::list<MouseMotionCallback>::const_iterator Input::addMouseMotionCallback(MouseMotionCallback callback)
{
    return s_mouseMotionCallbacks.insert(s_mouseMotionCallbacks.end(), callback);
}

std::list<MouseWheelCallback>::const_iterator Input::addMouseWheelUpCallback(MouseWheelCallback callback)
{
    return s_scrollWheelUpCallbacks.insert(s_scrollWheelUpCallbacks.end(), callback);
}

std::list<MouseWheelCallback>::const_iterator Input::addMouseWheelDownCallback(MouseWheelCallback callback)
{
    return s_scrollWheelDownCallbacks.insert(s_scrollWheelDownCallbacks.end(), callback);
}


void Input::removeMouseButtonPressCallback(std::list<ButtonPressCallback>::const_iterator referenceToCallback, Button::ButtonCode button)
{
    s_buttonPressCallbacks[button].erase(referenceToCallback);
}

void Input::removeMouseButtonReleaseCallback(std::list<ButtonReleaseCallback>::const_iterator referenceToCallback, Button::ButtonCode button)
{
    s_buttonReleaseCallbacks[button].erase(referenceToCallback);
}

void Input::removeKeyPressCallback(std::list<KeyPressCallback>::const_iterator referenceToCallback, Key::Keycode key)
{
    s_keyPressCallbacks[key].erase(referenceToCallback);
}

void Input::removeKeyReleaseCallback(std::list<KeyReleaseCallback>::const_iterator referenceToCallback, Key::Keycode key)
{
    s_keyReleaseCallbacks[key].erase(referenceToCallback);
}

void Input::removeMouseMotionCallback(std::list<MouseMotionCallback>::const_iterator referenceToCallback)
{
    s_mouseMotionCallbacks.erase(referenceToCallback);
}

void Input::removeMouseWheelUpCallback(std::list<MouseWheelCallback>::const_iterator referenceToCallback)
{
    s_scrollWheelUpCallbacks.erase(referenceToCallback);
}

void Input::removeMouseWheelDownCallback(std::list<MouseWheelCallback>::const_iterator referenceToCallback)
{
    s_scrollWheelDownCallbacks.erase(referenceToCallback);
}

