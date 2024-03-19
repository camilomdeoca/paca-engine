#include "Window.hpp"

#include <GL/glew.h>
#include <SDL2/SDL_video.h>
#include <cstdio>

Window::Window()
{}

Window::~Window()
{}

void Window::create(std::string title, int w, int h)
{
    m_width = w;
    m_height = h;
    m_window.reset(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL));

    if (!m_window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        exit(1);
    }

    const SDL_GLContext glContext = SDL_GL_CreateContext(m_window.get());
    if (!glContext) {
        fprintf(stderr, "Error creating GL context: %s\n", SDL_GetError());
        exit(1);
    }

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Error initializing glew.\n");
        exit(1);
    }

    SDL_GL_SetSwapInterval(1);
}

void Window::swapBuffers()
{
    SDL_GL_SwapWindow(m_window.get());
}

