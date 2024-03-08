#pragma once

#include "OrthoCamera.hpp"
class Renderer2D {
public:
    static void init();
    
    static void beginScene(const OrthoCamera &camera);
    static void endScene();

    static void drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);
};
