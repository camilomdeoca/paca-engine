#include "game/Action.hpp"

#include "utils/Assert.hpp"
#include "game/Input.hpp"

#include <memory>
#include <unordered_set>

// using is_transparent for searching for action with name without duplicating the action name as
// the unordered_map key
struct ActionHash {
    using is_transparent = void;
    size_t operator()(Action* const& action) const {
        return std::hash<std::string_view>{}(action->getName());
    }
    size_t operator()(const std::string_view &name) const {
        return std::hash<std::string_view>{}(name);
    }
};

struct ActionEqual {
    using is_transparent = void;
    bool operator()(Action* const &action1, Action* const &action2) const
    {
        return std::equal_to<std::string_view>{}(action1->getName(), action2->getName());
    }
    bool operator()(const std::string_view &name, Action* const &action) const
    {
        return std::equal_to<std::string_view>{}(action->getName(), name);
    }
    bool operator()(Action* const &action, const std::string_view &name) const
    {
        return std::equal_to<std::string_view>{}(action->getName(), name);
    }
};

static struct {
    std::unique_ptr<EventReceiver> eventReceiver;
    std::unordered_set<Action*, ActionHash, ActionEqual> actions;
    std::array<Action*, Key::last> keyBindings;
    std::array<Action*, Button::last> buttonBindings;
    Action *mouseWheelDownBinding, *mouseWheelUpBinding;
} s_data;

Action::Action()
{}

Action::~Action()
{
    s_data.actions.erase(this);

    // This deletes all bindings that use the action
    // TODO: find something better to not iterate through the bindings
    // maybe store all keys the action is bound to, but i dont like it
    auto keyIter = s_data.keyBindings.begin();
    while (keyIter != s_data.keyBindings.end())
    {
        if (*keyIter == this)
            *keyIter = nullptr;
        keyIter++;
    }

    auto buttonIter = s_data.buttonBindings.begin();
    while (buttonIter != s_data.buttonBindings.end())
    {
        if (*buttonIter == this)
            *buttonIter = nullptr;
        buttonIter++;
    }

    if (s_data.mouseWheelDownBinding == this)
        s_data.mouseWheelDownBinding = nullptr;

    if (s_data.mouseWheelUpBinding == this)
        s_data.mouseWheelUpBinding = nullptr;
}

void Action::init(std::string_view name, std::function<void()> enable, std::function<void()> disable)
{
    m_name = name;
    m_enable = enable;
    m_disable = disable;
    s_data.actions.insert(this);
}

void Action::exec(bool enable)
{
    if (enable)
    {
        if (m_enable) m_enable();
        ASSERT_MSG(m_enable, "Action has no functions");
    }
    else
    {
        if (m_disable) m_disable();
    }
}

void BindingsManager::init()
{
    s_data.eventReceiver = std::make_unique<EventReceiver>();
    s_data.eventReceiver->setEventHandler([] (const Event &event) {
        switch (event.getType()) {
        case EventType::keyDown:
            {
                const KeyEvent &keyEvent = static_cast<const KeyEvent&>(event);
                Action *&action = s_data.keyBindings[keyEvent.getKey()];
                if (action) action->exec(true);
                break;
            }
        case EventType::keyUp:
            {
                const KeyEvent &keyEvent = static_cast<const KeyEvent&>(event);
                Action *&action = s_data.keyBindings[keyEvent.getKey()];
                if (action) action->exec(false);
                break;
            }
        case EventType::buttonDown:
            {
                const ButtonEvent &buttonEvent = static_cast<const ButtonEvent&>(event);
                Action *&action = s_data.buttonBindings[buttonEvent.getButton()];
                if (action) action->exec(true);
                break;
            }
        case EventType::buttonUp:
            {
                const ButtonEvent &buttonEvent = static_cast<const ButtonEvent&>(event);
                Action *&action = s_data.buttonBindings[buttonEvent.getButton()];
                if (action) action->exec(false);
                break;
            }
        case EventType::mouseWheelDown:
            {
                const MouseWheelEvent &wheelEvent = static_cast<const MouseWheelEvent&>(event);
                if (s_data.mouseWheelDownBinding)
                    for (int i = 0; i < -wheelEvent.getAmmount(); i++)
                        s_data.mouseWheelDownBinding->exec();
                break;
            }
        case EventType::mouseWheelUp:
            {
                const MouseWheelEvent &wheelEvent = static_cast<const MouseWheelEvent&>(event);
                if (s_data.mouseWheelUpBinding)
                    for (int i = 0; i < wheelEvent.getAmmount(); i++)
                        s_data.mouseWheelUpBinding->exec();
                break;
            }
        default:
            ASSERT_MSG(false, "Event of unrequested type arrived.");
            break;
        }
    });
    s_data.eventReceiver->setEventsMask(
            EventMask::keyDown |
            EventMask::keyUp |
            EventMask::buttonDown |
            EventMask::buttonUp |
            EventMask::mouseWheelDown |
            EventMask::mouseWheelUp);
}

void BindingsManager::shutdown()
{
    s_data.eventReceiver.reset();
}

bool BindingsManager::bind(Key::KeyCode key, std::string_view actionName)
{
    auto it = s_data.actions.find(actionName);
    if (it == s_data.actions.end())
        return false;
    s_data.keyBindings[key] = *it;
    return true;
}

bool BindingsManager::bind(Button::ButtonCode button, std::string_view actionName)
{
    auto it = s_data.actions.find(actionName);
    if (it == s_data.actions.end())
        return false;
    s_data.buttonBindings[button] = *it;
    return true;
}

bool BindingsManager::bindMouseWheelUp(std::string_view actionName)
{
    auto it = s_data.actions.find(actionName);
    if (it == s_data.actions.end())
        return false;
    s_data.mouseWheelUpBinding = *it;
    return true;
}

bool BindingsManager::bindMouseWheelDown(std::string_view actionName)
{
    auto it = s_data.actions.find(actionName);
    if (it == s_data.actions.end())
        return false;
    s_data.mouseWheelDownBinding = *it;
    return true;
}
