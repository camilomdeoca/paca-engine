#include "Renderer.hpp"

#include "engine/Material.hpp"
#include "opengl/FrameBuffer.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/Texture.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/gl.hpp"
#include "opengl/Shader.hpp"

#include <cstdint>
#include <memory>
#include <string>

static struct {
    std::shared_ptr<Shader> shader;
    
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    std::shared_ptr<FrameBuffer> framebuffer;
    std::shared_ptr<Texture> colorTexture, depthStencilTexture;
    
    std::shared_ptr<Shader> postProcessingShader;
    std::shared_ptr<VertexArray> quadForPostProcessingVertexArray;
    glm::mat3 kernel = {
        -1.0f, -1.0f, -1.0f,
        -1.0f,  9.0f, -1.0f,
        -1.0f, -1.0f, -1.0f
    };
    //glm::mat3 kernel = {
    //    0.0f, 0.0f, 0.0f,
    //    0.0f, 1.0f, 0.0f,
    //    0.0f, 0.0f, 0.0f
    //};
} s_data;

void Renderer::init()
{
    s_data.shader = std::make_shared<Shader>("assets/shaders/vertexNoUV.glsl", "assets/shaders/fragmentNoUV.glsl");
    FrameBufferParameters frameBufferParameters;
    frameBufferParameters.width = 1600;
    frameBufferParameters.height = 900;
    frameBufferParameters.textureAttachmentFormats = {
        Texture::Format::RGBA8, Texture::Format::depth24stencil8
    };
    s_data.framebuffer = std::make_shared<FrameBuffer>(frameBufferParameters);
    s_data.colorTexture = s_data.framebuffer->getColorAttachments()[0];
    s_data.depthStencilTexture = s_data.framebuffer->getDepthAttachment();

    s_data.postProcessingShader = std::make_shared<Shader>("assets/shaders/vertexPostProcess.glsl","assets/shaders/fragmentPostProcess.glsl");
    s_data.quadForPostProcessingVertexArray = std::make_shared<VertexArray>();

    float quadVertices[] = {
        // positions   // texCoords
        -1.0f, -1.0f,  0.0f, 0.0f, // bl
         1.0f, -1.0f,  1.0f, 0.0f, // br
         1.0f,  1.0f,  1.0f, 1.0f, // tr
        -1.0f,  1.0f,  0.0f, 1.0f, // tl
    };
    uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };

    std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>(quadVertices, sizeof(quadVertices));
    vertexBuffer->setLayout({
        {ShaderDataType::float2, "a_position"},
        {ShaderDataType::float2, "a_uv"}
    });
    s_data.quadForPostProcessingVertexArray->addVertexBuffer(vertexBuffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices, sizeof(indices) / sizeof(uint32_t));
    s_data.quadForPostProcessingVertexArray->setIndexBuffer(indexBuffer);
}

void Renderer::beginScene(const Camera &camera)
{
    s_data.framebuffer->bind();
    GL::clear();
    GL::setDepthTest(true);
    s_data.shader->bind();
    s_data.projectionMatrix = camera.getProjectionMatrix();
    s_data.viewMatrix = camera.getViewMatrix();
    s_data.shader->setMat4("u_projectionMatrix", s_data.projectionMatrix);
    s_data.shader->setMat4("u_viewModelMatrix", s_data.viewMatrix); // TODO: Change when we are able to move models
    s_data.shader->setFloat3("u_viewPosition", camera.getPosition());
}

void Renderer::endScene()
{
    s_data.framebuffer->unbind();
    GL::clear();
    s_data.postProcessingShader->bind();
    GL::setDepthTest(false);
    s_data.colorTexture->bind(0);
    s_data.postProcessingShader->setFloat("u_screenWidth", 1600);
    s_data.postProcessingShader->setFloat("u_screenHeight", 900);
    s_data.postProcessingShader->setMat3("u_kernel", s_data.kernel);
    s_data.postProcessingShader->setInt("u_screenTexture", 0);
    s_data.quadForPostProcessingVertexArray->bind();
    GL::drawIndexed(s_data.quadForPostProcessingVertexArray);
}

void Renderer::drawMesh(Mesh &mesh)
{
    s_data.shader->bind();
    //s_data.shader->setFloat4("u_color", {1.0f, 0.8f, 0.7f, 1.0f});

    unsigned int i = 0;
    for (const std::shared_ptr<Texture> &texture : mesh.getMaterial()->getTextures(MaterialTextureType::diffuse))
    {
        texture->bind(i);
        s_data.shader->setInt("u_diffuse" + std::to_string(i), i);
        i++;
    }
    for (const std::shared_ptr<Texture> &texture : mesh.getMaterial()->getTextures(MaterialTextureType::specular))
    {
        texture->bind(i);
        s_data.shader->setInt("u_specular" + std::to_string(i), i);
        i++;
    }

    mesh.getVertexArray()->bind();
    GL::drawIndexed(mesh.getVertexArray());
}

void Renderer::drawModel(Model &model)
{
    for (const std::shared_ptr<Mesh> &mesh : model.getMeshes())
    {
        drawMesh(*mesh);
    }
}

