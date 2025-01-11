#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_scancode.h>
#include <functional>
#include <glm/glm.hpp>
#include <utility>
#include <variant>

namespace engine::input {

enum class Key {
    q      = SDL_SCANCODE_Q,
    w      = SDL_SCANCODE_W,
    e      = SDL_SCANCODE_E,
    a      = SDL_SCANCODE_A,
    s      = SDL_SCANCODE_S,
    d      = SDL_SCANCODE_D,
    f      = SDL_SCANCODE_F,
    b      = SDL_SCANCODE_B,
    right  = SDL_SCANCODE_RIGHT,
    left   = SDL_SCANCODE_LEFT,
    down   = SDL_SCANCODE_DOWN,
    up     = SDL_SCANCODE_UP,
    lshift = SDL_SCANCODE_LSHIFT,
    space  = SDL_SCANCODE_SPACE,
    esc    = SDL_SCANCODE_ESCAPE,

    last   = SDL_NUM_SCANCODES
};

enum class Button {
    left = SDL_BUTTON_LEFT,
    middle = SDL_BUTTON_MIDDLE,
    right = SDL_BUTTON_RIGHT,

    last
};

struct ButtonEvent {
    Button button;
};

struct ButtonPressEvent : public ButtonEvent {};

struct ButtonReleaseEvent : public ButtonEvent {};

struct KeyEvent {
    Key key;
};

struct KeyPressEvent : public KeyEvent {};

struct KeyReleaseEvent : public KeyEvent {};

struct MouseMotionEvent {
    glm::ivec2 position;
    glm::ivec2 relativePosition;
};

struct MouseWheelEvent {
    int amount;
};

struct MouseWheelDownEvent : public MouseWheelEvent {};

struct MouseWheelUpEvent : public MouseWheelEvent {};

struct ResizeEvent {
    unsigned int w, h;
};

struct ExitEvent {};

enum class EventMask {
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

inline constexpr EventMask operator|(const EventMask &e1, const EventMask &e2)
{
    return EventMask(std::to_underlying(e1) | std::to_underlying(e2));
}

using Event = std::variant<
    ButtonPressEvent,
    ButtonReleaseEvent,
    KeyPressEvent,
    KeyReleaseEvent,
    MouseMotionEvent,
    MouseWheelDownEvent,
    MouseWheelUpEvent,
    ResizeEvent,
    ExitEvent
>;

class EventReceiver {
public:
    EventReceiver();
    ~EventReceiver();

    void setEventsMask(EventMask eventsMask);
    void setEventHandler(std::function<void(const Event&)> handler) { m_eventHandler = handler; }
    void onEvent(const Event &event) { m_eventHandler(event); }
private:
    std::function<void(const Event&)> m_eventHandler;
    uint32_t m_eventsMask = 0;
};

class Manager {
public:
    void init();

    void registerEventReceiver();

private:
    std::array<std::vector<EventReceiver*>, std::variant_size_v<Event>> m_receivers;
};

}
