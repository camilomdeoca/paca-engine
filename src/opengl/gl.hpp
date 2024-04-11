#pragma once

#include "VertexArray.hpp"
#include <glm/glm.hpp>
#include <memory>

class GL {
public:
    enum class BlendFunction {
        one,
        srcAlpha,
        oneMinusSrcAlpha,

        last
    };

    enum class DepthTestFunction {
        less,
        lessEqual,

        last
    };

    static void init();

    static void setClearColor(const glm::vec4 &color);
    static void clear();
    static void drawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0);
    static void setDepthTest(bool value);
    static void setDepthTestFunction(DepthTestFunction function);
    static void setBlending(bool value);
    static void setBlendFunction(BlendFunction src, BlendFunction dst);
    static void viewport(unsigned int width, unsigned int height);
};

