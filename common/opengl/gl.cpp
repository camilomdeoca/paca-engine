#include "opengl/gl.hpp"

#include <glm/glm.hpp>

#include <GL/glew.h>
#include <cstddef>

void GL::init()
{
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
	//glEnable(GL_LINE_SMOOTH);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // if textures are not 4-byte aligned (default)
}

void GL::setClearColor(const glm::vec4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
}

void GL::clear()
{
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GL::drawIndexed(const VertexArray &vertexArray, uint32_t indexCount)
{
    vertexArray.bind();
    uint32_t count = indexCount ? indexCount : vertexArray.getIndexBuffer()->getCount();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void GL::drawLines(const VertexArray &vertexArray, uint32_t indexCount)
{
    vertexArray.bind();
    uint32_t count = indexCount ? indexCount : vertexArray.getIndexBuffer()->getCount();
    glDrawElements(GL_LINES, count, GL_UNSIGNED_INT, nullptr);
}

void GL::drawPoints(const VertexArray &vertexArray, uint32_t indexCount)
{
    glPointSize(6.0);
    vertexArray.bind();
    uint32_t count = indexCount ? indexCount : vertexArray.getIndexBuffer()->getCount();
    glDrawElements(GL_POINTS, count, GL_UNSIGNED_INT, nullptr);
}

void GL::setDepthTest(bool value)
{
    if (value)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void GL::setDepthTestFunction(DepthTestFunction function)
{
    constexpr GLenum depthTestFuncToGLDepthFunc[static_cast<size_t>(DepthTestFunction::last)] = {
        GL_LESS,
        GL_LEQUAL,
        GL_ALWAYS
    };
    glDepthFunc(depthTestFuncToGLDepthFunc[static_cast<size_t>(function)]);
}

void GL::setBlending(bool value)
{
    if (value)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
}

void GL::setBlendFunction(BlendFunction src, BlendFunction dst)
{
    constexpr GLenum blendFuncToGLBlendFunc[static_cast<size_t>(BlendFunction::last)] = {
        GL_ONE,
        GL_SRC_ALPHA,
        GL_ONE_MINUS_SRC_ALPHA
    };

    glBlendFunc(blendFuncToGLBlendFunc[static_cast<size_t>(src)],
                blendFuncToGLBlendFunc[static_cast<size_t>(dst)]);
}

void GL::viewport(unsigned int width, unsigned int height)
{
    glViewport(0, 0, width, height);
}

void GL::viewport(int x, int y, unsigned int width, unsigned int height)
{
    glViewport(x, y, width, height);
}

void GL::setPolygonMode(PolygonMode mode)
{
    constexpr GLenum polygonModeToGLPolygonMode[static_cast<size_t>(PolygonMode::last)] = {
        GL_FILL,
        GL_LINE
    };

    glPolygonMode(GL_FRONT, polygonModeToGLPolygonMode[static_cast<size_t>(mode)]);
}
