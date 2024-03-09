#pragma once

#include "OrthoCamera.hpp"
#include "opengl/Texture.hpp"

#include <memory>
#include <string>

class Renderer2D {
public:
    static void init();
    
    static void beginScene(const OrthoCamera &camera);
    static void endScene();

    static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
    static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, std::shared_ptr<Texture> texture, const glm::vec4 &color);
    static void drawString(const glm::vec3 &position, const std::string &text, const glm::vec4 &color);
};
