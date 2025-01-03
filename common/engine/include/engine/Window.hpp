#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <string>

class Window {
public:
    Window();
    ~Window();

    void create(std::string title = "Window Title", int w = 1600, int h = 900);

    unsigned int getWidth() { return m_width; }
    unsigned int getHeight() { return m_height; }

    void swapBuffers();

    SDL_Window *getSDLWindow() const { return m_window.get(); }
    SDL_GLContext getSDLGLContext() const { return m_glContext; }

private:
    struct deleter {
        void operator()(SDL_Window *window){
            SDL_DestroyWindow(window);
        }
    };

    std::unique_ptr<SDL_Window, deleter> m_window;
    unsigned int m_width = 0, m_height = 0;
    SDL_GLContext m_glContext;
};
