#include "Action.hpp"

#include "engine/Assert.hpp"
#include "engine/Input.hpp"

#include <memory>
#include <unordered_map>
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
    bool operator()(const Action* const& action1, Action* const& action2) const
    {
        return std::equal_to<std::string_view>{}(action1->getName(), action2->getName());
    }
    bool operator()(const std::string_view &name1, const std::string_view &name2) const
    {
        return std::equal_to<std::string_view>{}(name1, name2);
    }
    bool operator()(const std::string_view &name, Action* const& action) const
    {
        return std::equal_to<std::string_view>{}(action->getName(), name);
    }
    bool operator()(Action* const& action, const std::string_view &name) const
    {
        return std::equal_to<std::string_view>{}(action->getName(), name);
    }
};

static struct {
    std::unique_ptr<EventReceiver> eventReceiver;
    std::unordered_set<Action*, ActionHash, ActionEqual> actions;
    std::unordered_map<Key::KeyCode, Action*> bindings;
} s_data;

Action::Action()
{}

Action::~Action()
{
    s_data.actions.erase(this);

    // This deletes all bindings that use the action
    // TODO: find something better to not iterate through the bindings
    // maybe store all keys the action is bound to, but i dont like it
    auto it = s_data.bindings.begin();
    while (it != s_data.bindings.end())
    {
        if (it->second == this)
            it = s_data.bindings.erase(it);
        else
            it++;
    }
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
                auto it = s_data.bindings.find(keyEvent.getKey());
                if (it != s_data.bindings.end())
                    it->second->exec(true);
                break;
            }
        case EventType::keyUp:
            {
                const KeyEvent &keyEvent = static_cast<const KeyEvent&>(event);
                auto it = s_data.bindings.find(keyEvent.getKey());
                if (it != s_data.bindings.end())
                    it->second->exec(false);
                break;
            }
        default:
            ASSERT_MSG(false, "Event of unrequested type arrived.");
            break;
        }
    });
    s_data.eventReceiver->setEventsMask(EventMask::keyDown | EventMask::keyUp);
}

void BindingsManager::shutdown()
{
    s_data.eventReceiver.reset();
}

bool BindingsManager::bind(Key::KeyCode key, std::string actionName)
{
    auto it = s_data.actions.find(actionName);
    if (it == s_data.actions.end())
    {
        return false;
    }
    s_data.bindings[key] = *it;
    return true;
}
