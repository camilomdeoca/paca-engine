#pragma once

#include "engine/Camera.hpp"
#include "engine/Font.hpp"
#include "opengl/Texture.hpp"

#include <glm/glm.hpp>
#include <string>

class Renderer2D {
public:
    static void init();
    
    static void beginScene(const Camera &camera);
    static void endScene();

    static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
    static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const Texture &texture, const glm::vec4 &color = glm::vec4(1.0f));
    static void drawString(const glm::vec3 &position, const std::string &text, Font &font, const glm::vec4 &color, float size = 1.0f);
};
