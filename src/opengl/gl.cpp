#include "gl.hpp"

#include <GL/glew.h>

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

void GL::drawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount)
{
    vertexArray->bind();
    uint32_t count = indexCount ? indexCount : vertexArray->getIndexBuffer()->getCount();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
}

void GL::setDepthTest(bool value)
{
    if (value)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void GL::setBlending(bool value)
{
    if (value)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

}
