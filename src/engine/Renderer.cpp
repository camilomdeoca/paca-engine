#include "Renderer.hpp"

#include "opengl/gl.hpp"
#include "opengl/Shader.hpp"

#include <memory>

static struct {
    std::shared_ptr<Shader> shader;
    glm::mat4 viewProjectionMatrix;
} s_data;

void Renderer::init()
{
    s_data.shader = std::make_shared<Shader>("assets/shaders/vertexNoUV.glsl", "assets/shaders/fragmentNoUV.glsl");
}

void Renderer::beginScene(const Camera &camera)
{
    s_data.shader->bind();
    s_data.viewProjectionMatrix = camera.getViewProjectionMatrix();
    s_data.shader->setMat4("u_viewProjection", s_data.viewProjectionMatrix);
}

void Renderer::endScene()
{}

void Renderer::drawMesh(Mesh &mesh)
{
    s_data.shader->bind();
    s_data.shader->setFloat4("u_color", {1.0f, 0.0f, 0.0f, 1.0f});

    mesh.getVertexArray()->bind();
    GL::drawIndexed(mesh.getVertexArray());
}

