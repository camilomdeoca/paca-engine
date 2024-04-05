#pragma once

#include "engine/Input.hpp"

#include <functional>
#include <string>

// TODO: remove action when it isnt valid anymore?
// If the camera controller registers actions for moving forward and backwards and then the camera
// controller object gets deleted the functions asociated with the actions will not be valid
// so what should i do?

class Action {
public:
    Action(std::string name, std::function<void()> action);
    ~Action();

    const std::string &getName() const { return m_name; }
    void exec() { m_action(); }

private:
    std::string m_name;
    std::function<void()> m_action;
};

class BindingsManager {
public:
    static void init();
    static void shutdown();
    static bool registerAction(Action &action); // if action already exists returns error, see if true or false
    static bool bindKeyToAction(Key::KeyCode key, std::string actionName);
};


