#include "Renderer2D.hpp"
#include "../opengl/VertexArray.hpp"
#include "../opengl/Shader.hpp"
#include "../opengl/gl.hpp"
#include "opengl/Texture.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <memory>

static struct {
    std::shared_ptr<VertexArray> quadsVertexArray;
    std::shared_ptr<Shader> quadsShader;

    std::shared_ptr<Texture> whiteTexture;
} s_data;

void Renderer2D::init()
{
    s_data.quadsVertexArray = std::make_shared<VertexArray>();

    float squareVertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };

    std::shared_ptr<VertexBuffer> quadVertexBuffer;
    quadVertexBuffer = std::make_shared<VertexBuffer>(squareVertices, sizeof(squareVertices));
    quadVertexBuffer->setLayout({
        {ShaderDataType::float3, "a_position"},
        {ShaderDataType::float2, "a_uv"}
    });
    s_data.quadsVertexArray->addVertexBuffer(quadVertexBuffer);
    
    uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };
    std::shared_ptr<IndexBuffer> quadIndexBuffer;
    quadIndexBuffer = std::make_shared<IndexBuffer>(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    s_data.quadsVertexArray->setIndexBuffer(quadIndexBuffer);

    unsigned char whitePixel[] = { 255, 255, 255, 255 };

    s_data.whiteTexture = std::make_shared<Texture>(whitePixel, 1, 1);

    s_data.quadsShader = std::make_shared<Shader>("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
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
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
        * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

    s_data.quadsShader->bind();
    s_data.quadsShader->setFloat4("u_color", color);
    s_data.quadsShader->setMat4("u_transform", transform);

    s_data.whiteTexture->bind(0);
    s_data.quadsShader->setInt("u_texture", 0);


    s_data.quadsVertexArray->bind();
    GL::drawIndexed(s_data.quadsVertexArray);
}

void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, std::shared_ptr<Texture> texture, const glm::vec4 &color)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
        * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

    s_data.quadsShader->bind();
    s_data.quadsShader->setFloat4("u_color", color);
    s_data.quadsShader->setMat4("u_transform", transform);

    texture->bind(0);
    s_data.quadsShader->setInt("u_texture", 0);

    s_data.quadsVertexArray->bind();
    GL::drawIndexed(s_data.quadsVertexArray);

}


void Renderer2D::drawString(const glm::vec3 &position, const std::string &text)
{
    
}
