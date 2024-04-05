#pragma once

#include "engine/Input.hpp"

#include <functional>
#include <string>
#include <string_view>

// TODO: remove action when it isnt valid anymore?
// If the camera controller registers actions for moving forward and backwards and then the camera
// controller object gets deleted the functions asociated with the actions will not be valid
// so what should i do?

// TODO: Create ToggleaAction class that has two functions one on enable and one on disable

class Action {
public:
    Action();
    ~Action();

    void init(std::string_view name, std::function<void()> enable, std::function<void()> disable = nullptr);

    std::string_view getName() const { return m_name; }
    void exec(bool enable);

private:
    std::string m_name;
    std::function<void()> m_enable, m_disable;
};

class BindingsManager {
public:
    static void init();
    static void shutdown();
    static bool bind(Key::KeyCode key, std::string actionName);
};


