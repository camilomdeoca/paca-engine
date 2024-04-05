#include "Window.hpp"

#include "engine/Assert.hpp"
#include "engine/Log.hpp"

#include <GL/glew.h>
#include <SDL2/SDL_video.h>

Window::Window()
{}

Window::~Window()
{
    if (m_glContext)
        SDL_GL_DeleteContext(m_glContext);
}

void Window::create(std::string title, int w, int h)
{
    m_width = w;
    m_height = h;
    m_window.reset(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));

    if (!m_window) {
        ERROR("Error creating window: {}", SDL_GetError());
        ASSERT(false);
    }

    m_glContext = SDL_GL_CreateContext(m_window.get());
    if (!m_glContext) {
        ERROR("Error creating GL context: {}", SDL_GetError());
        ASSERT(false);
    }

    if (glewInit() != GLEW_OK) {
        ERROR("Error initializing glew.");
        ASSERT(false);
    }

    SDL_GL_SetSwapInterval(1);
}

void Window::swapBuffers()
{
    SDL_GL_SwapWindow(m_window.get());
}

