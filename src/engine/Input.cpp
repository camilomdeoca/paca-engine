#include "Input.hpp"

#include <SDL2/SDL.h>
#include <array>
#include <set>

static const Uint8 *s_keyboardState = nullptr;

struct Pair { uint32_t mask; EventType type; };

constexpr std::array<Pair, static_cast<size_t>(EventType::last)> maskEnumMapping {{
    {EventMask::buttonDown, EventType::buttonDown},
    {EventMask::buttonUp, EventType::buttonUp},
    {EventMask::keyDown, EventType::keyDown},
    {EventMask::keyUp, EventType::keyUp},
    {EventMask::mouseMotion, EventType::mouseMotion},
    {EventMask::mouseWheelDown, EventType::mouseWheelDown},
    {EventMask::mouseWheelUp, EventType::mouseWheelUp},
    {EventMask::windowResize, EventType::windowResize}
}};

static struct {
    std::array<std::set<EventReceiver*>, static_cast<size_t>(EventType::last)> eventReceivers;
} s_data;

EventReceiver::EventReceiver()
{}

EventReceiver::~EventReceiver()
{
    for (const auto &[bit, type] : maskEnumMapping)
        if (m_eventsMask & bit)
            s_data.eventReceivers[static_cast<size_t>(type)].erase(this);
}

void EventReceiver::setEventsMask(uint32_t eventsMask)
{
    uint32_t changedBits = m_eventsMask ^ eventsMask;
    uint32_t changeDirection = changedBits & eventsMask; // 1 if changed from 0 to 1
    m_eventsMask = eventsMask;

    for (const auto &[bit, type] : maskEnumMapping)
        if (changedBits & bit)
        {
            if (changeDirection & bit)
                s_data.eventReceivers[static_cast<size_t>(type)].emplace(this);
            else
                s_data.eventReceivers[static_cast<size_t>(type)].erase(this);
        }
}

void Input::init()
{
    s_keyboardState = SDL_GetKeyboardState(nullptr);
}

void sendEventToReceiversForType(const Event &event, EventType type)
{
    for (EventReceiver *receiver : s_data.eventReceivers[static_cast<size_t>(type)])
        receiver->onEvent(event);
}

void Input::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            // TODO: Cleanup before exiting
            exit(0);
            break;
        case SDL_WINDOWEVENT:
            {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                    {
                        ResizeEvent resizeEvent = {
                            static_cast<unsigned int>(event.window.data1),
                            static_cast<unsigned int>(event.window.data2)
                        };
                        sendEventToReceiversForType(resizeEvent, EventType::windowResize);
                    }
                    break;
                }
            }
            break;
        case SDL_KEYDOWN:
            {
                KeyPressEvent keyPressEvent = {
                    Key::KeyCode(event.key.keysym.scancode)
                };
                sendEventToReceiversForType(keyPressEvent, EventType::keyDown);
            }
            break;
        case SDL_KEYUP:
            {
                KeyReleaseEvent keyReleaseEvent = {
                    Key::KeyCode(event.key.keysym.scancode)
                };
                sendEventToReceiversForType(keyReleaseEvent, EventType::keyUp);
            }
            break;
        case SDL_MOUSEMOTION:
            {
                MouseMotionEvent mouseMotionEvent = {
                    event.motion.x, event.motion.y,
                    event.motion.xrel, event.motion.yrel
                };
                sendEventToReceiversForType(mouseMotionEvent, EventType::mouseMotion);
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            {
                ButtonPressEvent buttonPressEvent = {
                    Button::ButtonCode(event.button.button)
                };
                sendEventToReceiversForType(buttonPressEvent, EventType::buttonDown);
            }
            break;
        case SDL_MOUSEBUTTONUP:
            {
                ButtonReleaseEvent buttonReleaseEvent = {
                    Button::ButtonCode(event.button.button)
                };
                sendEventToReceiversForType(buttonReleaseEvent, EventType::buttonUp);
            }
            break;
        case SDL_MOUSEWHEEL:
            {
                if (event.wheel.y < 0)
                {
                    MouseWheelDownEvent mouseWheelEvent = {
                        event.wheel.y
                    };
                    sendEventToReceiversForType(mouseWheelEvent, EventType::mouseWheelDown);
                }
                else
                {
                    MouseWheelUpEvent mouseWheelEvent = {
                        event.wheel.y
                    };
                    sendEventToReceiversForType(mouseWheelEvent, EventType::mouseWheelUp);
                }
            }
            break;
        }
    }
}

void Input::restrainMouseToWindow(bool enabled)
{
    SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
}

bool Input::isKeyPressed(Key::KeyCode key)
{
    return s_keyboardState[key];
}

bool Input::isMouseButtonPressed(Button::ButtonCode button)
{
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(button);
}

