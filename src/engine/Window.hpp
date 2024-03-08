#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>

class Window {
public:
    Window();
    ~Window();

    void create(std::string title = "Window Title", int w = 1600, int h = 900);

    void swapBuffers();
private:
    struct deleter {
        void operator()(SDL_Window *window){
            SDL_DestroyWindow(window);
        }
    };

    std::unique_ptr<SDL_Window, deleter> m_window;
};
