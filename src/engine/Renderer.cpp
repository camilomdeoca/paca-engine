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
#include <cstdio>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <string>
#include <vector>

static struct {
    unsigned int width, height;
    std::shared_ptr<Shader> shader;
    
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 cameraPos;

    const std::vector<std::shared_ptr<Light>> *lights;

    std::shared_ptr<FrameBuffer> gBuffer;
    std::shared_ptr<Texture> gNormalTexture, gAlbedoSpecularTexture, gDepthTexture;
    std::shared_ptr<Shader> gBufferShader;

    std::shared_ptr<Shader> lightPassShader;
    std::shared_ptr<Shader> lightShader;

    std::shared_ptr<FrameBuffer> postprocessFramebuffer;
    std::shared_ptr<Texture> postprocessColorTexture, postprocessDepthStencilTexture;
    
    std::shared_ptr<Shader> postProcessingShader;
    std::shared_ptr<VertexArray> fullscreenQuad;
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

std::string textureTypeToUniformName(MaterialTextureType::Type type)
{
    switch (type) {
        case MaterialTextureType::diffuse: return "u_diffuseMap";
        case MaterialTextureType::specular: return "u_specularMap";
        case MaterialTextureType::normal: return "u_normalMap";
        case MaterialTextureType::height: return "u_heightMap";
        default: break;
    }

    fprintf(stderr, "Invalid Material Texture Type!\n");
    exit(1);
}

void Renderer::init(RendererParameters parameters)
{
    s_data.shader = std::make_shared<Shader>("assets/shaders/vertexNoUV.glsl", "assets/shaders/fragmentNoUV.glsl");
    s_data.width = parameters.width;
    s_data.height = parameters.height;

    // G-Buffer
    FrameBufferParameters gBufferParams;
    gBufferParams.width = s_data.width;
    gBufferParams.height = s_data.height;
    gBufferParams.textureAttachmentFormats = {
        Texture::Format::RGBA16F, Texture::Format::RGBA8, Texture::Format::depth24stencil8
    };
    s_data.gBuffer = std::make_shared<FrameBuffer>(gBufferParams);
    s_data.gNormalTexture = s_data.gBuffer->getColorAttachments()[0];
    s_data.gAlbedoSpecularTexture = s_data.gBuffer->getColorAttachments()[1];
    s_data.gDepthTexture = s_data.gBuffer->getDepthAttachment();
    s_data.gBufferShader = std::make_shared<Shader>("assets/shaders/gBufferVertex.glsl", "assets/shaders/gBufferFragment.glsl");

    // Light pass
    s_data.lightPassShader = std::make_shared<Shader>("assets/shaders/lightPassVertex.glsl", "assets/shaders/lightPassFragment.glsl");
    s_data.lightShader = std::make_shared<Shader>("assets/shaders/lightShaderVertex.glsl", "assets/shaders/lightShaderFragment.glsl");

    // Post-process buffer
    FrameBufferParameters frameBufferParameters;
    frameBufferParameters.width = s_data.width;
    frameBufferParameters.height = s_data.height;
    frameBufferParameters.textureAttachmentFormats = {
        Texture::Format::RGBA8, Texture::Format::depth24stencil8
    };
    s_data.postprocessFramebuffer = std::make_shared<FrameBuffer>(frameBufferParameters);
    s_data.postprocessColorTexture = s_data.postprocessFramebuffer->getColorAttachments()[0];
    s_data.postprocessDepthStencilTexture = s_data.postprocessFramebuffer->getDepthAttachment();
    s_data.postProcessingShader = std::make_shared<Shader>("assets/shaders/vertexPostProcess.glsl","assets/shaders/fragmentPostProcess.glsl");

    // Full screen quad for lighting pass and postprocessing
    s_data.fullscreenQuad = std::make_shared<VertexArray>();
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
    s_data.fullscreenQuad->addVertexBuffer(vertexBuffer);

    std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indices, sizeof(indices) / sizeof(uint32_t));
    s_data.fullscreenQuad->setIndexBuffer(indexBuffer);
}

void Renderer::beginScene(const Camera &camera, const RenderEnvironment &environment)
{
    s_data.lights = environment.pointLights;

    // set up to render to G-Buffer
    s_data.gBuffer->bind();
    GL::setBlending(false);
    GL::setClearColor({0.0f, 0.0f, 0.0f, 1.0f});
    GL::clear();
    GL::setDepthTest(true);
    s_data.gBufferShader->bind();
    s_data.projectionMatrix = camera.getProjectionMatrix();
    s_data.viewMatrix = camera.getViewMatrix();
    s_data.cameraPos = camera.getPosition();
    s_data.gBufferShader->setMat4("u_projectionMatrix", s_data.projectionMatrix);

    //
    //s_data.framebuffer->bind();
    //GL::clear();
    //GL::setDepthTest(true);
    //s_data.shader->bind();
    //s_data.projectionMatrix = camera.getProjectionMatrix();
    //s_data.viewMatrix = camera.getViewMatrix();
    //s_data.shader->setMat4("u_projectionMatrix", s_data.projectionMatrix);
    //s_data.shader->setMat4("u_viewModelMatrix", s_data.viewMatrix); // TODO: Change when we are able to move models
}

void Renderer::endScene()
{
    // TODO: Break each pass to a new function

    // light Pass
    s_data.postprocessFramebuffer->bind();
    GL::clear();
    s_data.lightPassShader->bind();
    s_data.gNormalTexture->bind(0);
    s_data.gAlbedoSpecularTexture->bind(1);
    s_data.gDepthTexture->bind(2);
    GL::setDepthTest(false);
    s_data.lightPassShader->setInt("u_gNormal", 0);
    s_data.lightPassShader->setInt("u_gColorSpec", 1);
    s_data.lightPassShader->setInt("u_gDepth", 2);
    s_data.lightPassShader->setMat4("u_inverseProjectionViewMatrix", glm::inverse(s_data.projectionMatrix));

    // Light parameters
    for (const std::shared_ptr<Light> &light : *s_data.lights)
    {
        glm::vec3 lightPosInViewSpace
            = s_data.viewMatrix
            * glm::vec4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 1.0f);

        s_data.lightPassShader->setFloat3("u_lights[0].posInViewSpace", lightPosInViewSpace);
        s_data.lightPassShader->setFloat3("u_lights[0].color", light->getColor());
        s_data.lightPassShader->setFloat("u_lights[0].intensity", light->getIntensity());
        s_data.lightPassShader->setFloat("u_lights[0].attenuation", light->getAttenuation());
        s_data.lightPassShader->setInt("u_numOfLights", 1);
        s_data.fullscreenQuad->bind();
        GL::drawIndexed(s_data.fullscreenQuad);
    }

    // Render Light models
    FrameBuffer::copy(*s_data.gBuffer, *s_data.postprocessFramebuffer);
    s_data.lightShader->bind();
    GL::setDepthTest(true);
    s_data.lightShader->setMat4("u_projectionMatrix", s_data.projectionMatrix);
    for (const std::shared_ptr<Light> &light : *s_data.lights)
    {
        if (light->getModel())
        {
            light->getModel()->setPosition(light->getPosition());

            // Put this into a function in Model class
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, light->getModel()->getPosition());
            transform = glm::scale(transform, light->getModel()->getScale());
            glm::quat rot(glm::radians(light->getModel()->getRotation()));
            transform = transform * glm::mat4_cast(rot);

            s_data.lightShader->setMat4("u_viewModelMatrix", s_data.viewMatrix * transform);
            s_data.lightShader->setFloat3("u_lightColor", light->getColor());
            for (const std::shared_ptr<Mesh> &mesh : light->getModel()->getMeshes())
            {
                mesh->getVertexArray()->bind();
                GL::drawIndexed(mesh->getVertexArray());
            }
        }
    }

    // Post-process
    s_data.postprocessFramebuffer->unbind();
    GL::setClearColor({0.1f, 0.1f, 0.15f, 1.0f});
    GL::clear();
    s_data.postProcessingShader->bind();
    GL::setDepthTest(false);
    s_data.postprocessColorTexture->bind(0);
    s_data.postProcessingShader->setFloat("u_screenWidth", s_data.width);
    s_data.postProcessingShader->setFloat("u_screenHeight", s_data.height);
    s_data.postProcessingShader->setMat3("u_kernel", s_data.kernel);
    s_data.postProcessingShader->setInt("u_screenTexture", 0);
    s_data.fullscreenQuad->bind();
    GL::drawIndexed(s_data.fullscreenQuad);
}

void Renderer::drawMesh(Mesh &mesh, const glm::mat4 &modelMatrix)
{
    s_data.gBufferShader->bind();
    s_data.gBufferShader->setMat4("u_viewModelMatrix", s_data.viewMatrix * modelMatrix);

    //s_data.shader->bind();
    //s_data.shader->setFloat4("u_color", {1.0f, 0.8f, 0.7f, 1.0f});
    //s_data.shader->setMat4("u_viewModelMatrix", s_data.viewMatrix * modelMatrix);

    unsigned int slot = 0;
    for (unsigned int i = 0; i < MaterialTextureType::last; i++) {
        unsigned int indexOfTextureOfType = 0;
        for (const std::shared_ptr<Texture> &texture : mesh.getMaterial()->getTextures(MaterialTextureType::Type(i)))
        {
            texture->bind(slot);
            s_data.gBufferShader->setInt(textureTypeToUniformName(MaterialTextureType::Type(i)) + std::to_string(indexOfTextureOfType), slot);
            s_data.shader->setInt(textureTypeToUniformName(MaterialTextureType::Type(i)) + std::to_string(indexOfTextureOfType), slot);
            slot++, indexOfTextureOfType++;
        }
    }

    mesh.getVertexArray()->bind();
    GL::drawIndexed(mesh.getVertexArray());
}

void Renderer::drawModel(Model &model)
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, model.getPosition());
    transform = glm::scale(transform, model.getScale());
    glm::quat rot(glm::radians(model.getRotation()));
    transform = transform * glm::mat4_cast(rot);

    for (const std::shared_ptr<Mesh> &mesh : model.getMeshes())
    {
        drawMesh(*mesh, transform);
    }
}

