#include "Renderer2D.hpp"
#include "../opengl/VertexArray.hpp"
#include "../opengl/Shader.hpp"
#include "../opengl/gl.hpp"

#include <memory>

static struct {
    std::shared_ptr<VertexArray> quadsVertexArray;
    std::shared_ptr<Shader> quadsShader;
} s_data;

void Renderer2D::init()
{
    s_data.quadsVertexArray = std::make_shared<VertexArray>();

    float squareVertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };

    std::shared_ptr<VertexBuffer> quadVertexBuffer;
    quadVertexBuffer = std::make_shared<VertexBuffer>(squareVertices, sizeof(squareVertices));
    quadVertexBuffer->setLayout({
        {ShaderDataType::float3, "a_position"}
    });
    s_data.quadsVertexArray->addVertexBuffer(quadVertexBuffer);
    
    uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };
    std::shared_ptr<IndexBuffer> quadIndexBuffer;
    quadIndexBuffer = std::make_shared<IndexBuffer>(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    s_data.quadsVertexArray->setIndexBuffer(quadIndexBuffer);

    s_data.quadsShader = std::make_shared<Shader>("assets/vertex.glsl", "assets/fragment.glsl");
}

void Renderer2D::beginScene(const OrthoCamera &camera)
{
    s_data.quadsShader->bind();
    s_data.quadsShader->setMat4("u_viewProjection", camera.getViewProjectionMatrix());
}

void Renderer2D::endScene()
{}

void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    s_data.quadsShader->bind();
    s_data.quadsShader->setFloat4("u_color", color);

    s_data.quadsVertexArray->bind();
    GL::drawIndexed(s_data.quadsVertexArray);
}
