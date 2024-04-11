#include "Renderer.hpp"

#include "engine/Assert.hpp"
#include "engine/Light.hpp"
#include "engine/Material.hpp"
#include "engine/PerspectiveCamera.hpp"
#include "opengl/FrameBuffer.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/Texture.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/VertexBuffer.hpp"
#include "opengl/gl.hpp"
#include "opengl/Shader.hpp"

#include <cstddef>
#include <cstdint>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <memory>
#include <string>
#include <vector>

struct ShadowMapLevel {
    // ProjectionView of the camera for the shadowMap
    glm::mat4 projectionView;

    // Frustum that the shadowMap level has to contain
    glm::mat4 frustumProjectionMatrix;

    std::shared_ptr<FrameBuffer> framebuffer;
    float near, far;
    float cutoffDistance;
    float maxDiagonal;
};

static struct {
    uint32_t width, height;
    
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 cameraPos;
    glm::mat4 prevProjectionMatrix;

    const std::vector<std::shared_ptr<PointLight>> *lights;
    std::shared_ptr<DirectionalLight> directionalLight;

    // Deferred rendering
    std::shared_ptr<FrameBuffer> gBuffer;
    std::shared_ptr<Texture> gNormalTexture, gAlbedoSpecularTexture, gDepthTexture;
    std::shared_ptr<Shader> gBufferShader;

    // Light pass
    std::shared_ptr<Shader> lightPassShader;
    std::shared_ptr<Shader> directionalLightPassShader;
    std::shared_ptr<Shader> lightModelsShader;

    // Shadow Mapping
    std::vector<ShadowMapLevel> shadowMap;
    std::shared_ptr<Shader> shadowMapShader;
    uint32_t shadowMapSize;

    // Post-process
    std::shared_ptr<FrameBuffer> postprocessFramebuffer;
    std::shared_ptr<Texture> postprocessColorTexture, postprocessDepthStencilTexture; 
    std::shared_ptr<Shader> postProcessingShader;

    std::shared_ptr<VertexArray> fullscreenQuad;
    //glm::mat3 kernel = {
    //    -1.0f, -1.0f, -1.0f,
    //    -1.0f,  9.0f, -1.0f,
    //    -1.0f, -1.0f, -1.0f
    //};
    glm::mat3 kernel = {
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };
} s_data;

// TODO: change return type to a std::array<glm::vec3, 8> because it always has 8 corners
std::array<glm::vec3, 8> getFrustumCorners(const glm::mat4 &projectionViewMatrix)
{
    const glm::mat4 inverseProjectionView = glm::inverse(projectionViewMatrix);
    std::array<glm::vec3, 8> frustumCorners;
    unsigned int nextIndex = 0;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 point = inverseProjectionView * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners[nextIndex++] = point / point.w;
            }
        }
    }
    return frustumCorners;
}

void Renderer::init(RendererParameters parameters)
{
    s_data.width = parameters.width;
    s_data.height = parameters.height;

    s_data.shadowMapSize = parameters.shadowMapSize;

    float prevSplit = 0.0f;
    s_data.shadowMap.resize(parameters.viewFrustumSplits.size());
    for (unsigned int i = 0; i < parameters.viewFrustumSplits.size(); i++)
    {
        s_data.shadowMap[i].near = prevSplit;
        s_data.shadowMap[i].far = parameters.viewFrustumSplits[i];
        prevSplit = parameters.viewFrustumSplits[i];
        s_data.shadowMap[i].cutoffDistance = parameters.viewFrustumSplits[i];
        INFO("SPLIT {} {}", s_data.shadowMap[i].near, s_data.shadowMap[i].far);
    }

    createFramebuffers();

    FrameBufferParameters shadowDepthMapBufferParams;
    shadowDepthMapBufferParams.width = s_data.shadowMapSize;
    shadowDepthMapBufferParams.height = s_data.shadowMapSize;
    shadowDepthMapBufferParams.textureAttachmentFormats = { Texture::Format::depth24 };
    for (ShadowMapLevel &shadowMapLevel : s_data.shadowMap)
    {
        shadowMapLevel.framebuffer = std::make_shared<FrameBuffer>(shadowDepthMapBufferParams);
        shadowMapLevel.framebuffer->getDepthAttachment()->setBorderColor({1.0f, 1.0f, 1.0f, 1.0f});
    }
    s_data.shadowMapShader = std::make_shared<Shader>("assets/shaders/shadowMapVertex.glsl", "assets/shaders/shadowMapFragment.glsl");

    s_data.gBufferShader = std::make_shared<Shader>("assets/shaders/gBufferVertex.glsl", "assets/shaders/gBufferFragment.glsl");
    s_data.lightPassShader = std::make_shared<Shader>("assets/shaders/lightPassVertex.glsl", "assets/shaders/lightPassFragment.glsl");
    s_data.directionalLightPassShader = std::make_shared<Shader>("assets/shaders/directionalLightPassVertex.glsl", "assets/shaders/directionalLightPassFragment.glsl");
    s_data.lightModelsShader = std::make_shared<Shader>("assets/shaders/lightShaderVertex.glsl", "assets/shaders/lightShaderFragment.glsl");
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

void Renderer::createFramebuffers()
{
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
    // Light pass buffers
    FrameBufferParameters frameBufferParameters;
    frameBufferParameters.width = s_data.width;
    frameBufferParameters.height = s_data.height;
    frameBufferParameters.textureAttachmentFormats = {
        Texture::Format::RGBA8, Texture::Format::depth24stencil8
    };
    s_data.postprocessFramebuffer = std::make_shared<FrameBuffer>(frameBufferParameters);
    s_data.postprocessColorTexture = s_data.postprocessFramebuffer->getColorAttachments()[0];
    s_data.postprocessDepthStencilTexture = s_data.postprocessFramebuffer->getDepthAttachment();
}

void Renderer::resize(uint32_t width, uint32_t height)
{
    s_data.width = width;
    s_data.height = height;
    createFramebuffers();
}

void Renderer::beginScene(const PerspectiveCamera &camera, const RenderEnvironment &environment)
{
    s_data.lights = environment.pointLights;
    s_data.directionalLight = environment.directionalLight;

    // set up to render to G-Buffer
    s_data.gBuffer->bind();
    GL::setBlending(false);
    GL::setClearColor({0.0f, 0.0f, 0.0f, 0.0f});
    GL::clear();
    GL::setDepthTest(true);
    s_data.gBufferShader->bind();
    s_data.projectionMatrix = camera.getProjectionMatrix();
    s_data.viewMatrix = camera.getViewMatrix();
    s_data.cameraPos = camera.getPosition();
    s_data.gBufferShader->setMat4("u_projectionMatrix", s_data.projectionMatrix);

    // Calculate view and projection (if the camera's projection changed) for redering each shadowMap
    float near = camera.getNear(), far = camera.getFar();
    static glm::mat4 previousProjectionMatrix = glm::mat4(0.0f);
    bool projectionChanged = previousProjectionMatrix != s_data.projectionMatrix;
    previousProjectionMatrix = s_data.projectionMatrix;
    for (ShadowMapLevel &shadowMap : s_data.shadowMap)
    {
        // Clear the shadowMap
        shadowMap.framebuffer->bind();
        GL::clear();

        // If the camera's projection changed the frustum that the shadowMap has to contain changed
        // so we need to recalculate it
        if (projectionChanged)
        {
            shadowMap.frustumProjectionMatrix = glm::perspective(
                    glm::radians(camera.getFov()), 
                    camera.getAspect(), 
                    std::max(shadowMap.near, near), 
                    std::min(shadowMap.far, far));
        }

        std::array<glm::vec3, 8> corners = getFrustumCorners(shadowMap.frustumProjectionMatrix * s_data.viewMatrix);
        ASSERT_MSG(corners.size() == 8, "There is {} corners instead of 8", corners.size());

        // calculate max diagonal of frustum if the projection changed
        if (projectionChanged)
        {
            shadowMap.maxDiagonal = 0.0f;
            for (const glm::vec3 &p1 : corners)
                for (const glm::vec3 &p2 : corners)
                    shadowMap.maxDiagonal = std::max(glm::distance(p1, p2), shadowMap.maxDiagonal);
        }

        // Calculate the center of the frustum
        glm::vec3 center = glm::vec3(0.0f);
        for (const glm::vec3 &p1 : corners)
            center += p1;
        center /= corners.size();

        const glm::vec3 &lightDirection = s_data.directionalLight->getDirection();
        glm::mat3 lightRotMatrix = glm::mat3(glm::lookAt(-lightDirection, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
        const float pixelSize = shadowMap.maxDiagonal / s_data.shadowMapSize;

        // Align coordinate axis to lightView so when the position of the shadowMap is rounded
        // It is rounded aligned with the pixels
        center = lightRotMatrix * center;
        center.x = std::round(center.x / pixelSize) * pixelSize;
        center.y = std::round(center.y / pixelSize) * pixelSize;
        center = glm::inverse(lightRotMatrix) * center;
        //center.z = std::round(center.z / pixelSize) * pixelSize;

        float &side = shadowMap.maxDiagonal;
        float depthOfShadowMap = 2.0f; // Tune this parameter
        shadowMap.projectionView = glm::ortho(-side/2, side/2, -side/2, side/2, -side*depthOfShadowMap, side*depthOfShadowMap)
                * glm::lookAt(center - lightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void lightPass()
{
    s_data.postprocessFramebuffer->bind();
    GL::clear();
    GL::setBlending(true);
    GL::setBlendFunction(GL::BlendFunction::one, GL::BlendFunction::one);
    s_data.lightPassShader->bind();
    s_data.gNormalTexture->bind(0);
    s_data.gAlbedoSpecularTexture->bind(1);
    s_data.gDepthTexture->bind(2);
    GL::setDepthTest(false);
    s_data.lightPassShader->setInt("u_gNormal", 0);
    s_data.lightPassShader->setInt("u_gColorSpec", 1);
    s_data.lightPassShader->setInt("u_gDepth", 2);
    s_data.lightPassShader->setMat4("u_inverseProjectionMatrix", glm::inverse(s_data.projectionMatrix));

    // Point lights
    for (const std::shared_ptr<PointLight> &light : *s_data.lights)
    {
        glm::vec3 lightPosInViewSpace
            = s_data.viewMatrix
            * glm::vec4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 1.0f);

        s_data.lightPassShader->setFloat3("u_lights[0].posInViewSpace", lightPosInViewSpace);
        s_data.lightPassShader->setFloat3("u_lights[0].color", light->getColor());
        s_data.lightPassShader->setFloat("u_lights[0].intensity", light->getIntensity());
        s_data.lightPassShader->setFloat("u_lights[0].attenuation", light->getAttenuation());
        s_data.lightPassShader->setInt("u_numOfLights", 1);
        GL::drawIndexed(s_data.fullscreenQuad);
    }

    // Directional Light
    if (s_data.directionalLight) {
        s_data.directionalLightPassShader->bind();
        s_data.gNormalTexture->bind(0);
        s_data.gAlbedoSpecularTexture->bind(1);
        s_data.gDepthTexture->bind(2);
        GL::setDepthTest(false);
        s_data.directionalLightPassShader->setInt("u_gNormal", 0);
        s_data.directionalLightPassShader->setInt("u_gColorSpec", 1);
        s_data.directionalLightPassShader->setInt("u_gDepth", 2);
        s_data.directionalLightPassShader->setMat4("u_inverseProjectionMatrix", glm::inverse(s_data.projectionMatrix));

        for (unsigned int i = 0; i < s_data.shadowMap.size(); i++)
        {
            ShadowMapLevel &shadowMap = s_data.shadowMap[i];
            shadowMap.framebuffer->getDepthAttachment()->bind(3 + i);
            s_data.directionalLightPassShader->setInt("u_shadowMaps[" + std::to_string(i) + "].texture", 3 + i);
            glm::mat4 cameraSpaceToLightSpace = shadowMap.projectionView * glm::inverse(s_data.viewMatrix);
            s_data.directionalLightPassShader->setMat4("u_shadowMaps[" + std::to_string(i) + "].cameraSpaceToLightSpace", cameraSpaceToLightSpace);
            s_data.directionalLightPassShader->setFloat("u_shadowMaps[" + std::to_string(i) + "].cutoffDistance", shadowMap.cutoffDistance);
        }
        s_data.directionalLightPassShader->setInt("u_numOfShadowMapLevels", s_data.shadowMap.size());

        s_data.directionalLightPassShader->setFloat3("u_light.directionInViewSpace", glm::mat3(s_data.viewMatrix) * s_data.directionalLight->getDirection());
        s_data.directionalLightPassShader->setFloat3("u_light.color", s_data.directionalLight->getColor());
        s_data.directionalLightPassShader->setFloat("u_light.intensity", s_data.directionalLight->getIntensity());
        GL::drawIndexed(s_data.fullscreenQuad);
    }

    GL::setBlendFunction(GL::BlendFunction::srcAlpha, GL::BlendFunction::oneMinusSrcAlpha);
}

void lightModelsPass()
{
    FrameBuffer::copy(*s_data.gBuffer, *s_data.postprocessFramebuffer);
    s_data.lightModelsShader->bind();
    GL::setDepthTest(true);
    s_data.lightModelsShader->setMat4("u_projectionMatrix", s_data.projectionMatrix);
    for (const std::shared_ptr<PointLight> &light : *s_data.lights)
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

            s_data.lightModelsShader->setMat4("u_viewModelMatrix", s_data.viewMatrix * transform);
            s_data.lightModelsShader->setFloat3("u_lightColor", light->getColor());
            for (const std::shared_ptr<Mesh> &mesh : light->getModel()->getMeshes())
            {
                mesh->getVertexArray()->bind();
                GL::drawIndexed(mesh->getVertexArray());
            }
        }
    }
}

void postProcessPass()
{
    s_data.postprocessFramebuffer->unbind();
    GL::setClearColor({0.1f, 0.1f, 0.15f, 1.0f});
    GL::clear();
    s_data.postProcessingShader->bind();
    GL::setDepthTest(false);
    s_data.postprocessColorTexture->bind(0);
    //s_data.shadowMap[0].framebuffer->getDepthAttachment()->bind(0);
    s_data.postProcessingShader->setMat3("u_kernel", s_data.kernel);
    s_data.postProcessingShader->setInt("u_screenTexture", 0);
    s_data.fullscreenQuad->bind();
    GL::drawIndexed(s_data.fullscreenQuad);
}

void Renderer::endScene()
{
    lightPass();
    lightModelsPass();
    postProcessPass();
}

std::string textureTypeToUniformName(MaterialTextureType::Type type)
{
    switch (type) {
        case MaterialTextureType::diffuse: return "u_diffuseMap";
        case MaterialTextureType::specular: return "u_specularMap";
        case MaterialTextureType::normal: return "u_normalMap";
        case MaterialTextureType::height: return "u_heightMap";
        default: break;
    }

    ASSERT_MSG(false, "Invalid Material Texture Type!");
}

std::string textureTypeToHasTextureUniformName(MaterialTextureType::Type type)
{
    switch (type) {
        case MaterialTextureType::diffuse: return "u_hasDiffuse";
        case MaterialTextureType::specular: return "u_hasSpecular";
        case MaterialTextureType::normal: return "u_hasNormal";
        case MaterialTextureType::height: return "u_hasHeight";
        default: break;
    }

    ASSERT_MSG(false, "Invalid Material Texture Type!");
}

void Renderer::drawMesh(Mesh &mesh, const glm::mat4 &modelMatrix)
{
    s_data.gBuffer->bind();
    s_data.gBufferShader->bind();
    s_data.gBufferShader->setMat4("u_viewModelMatrix", s_data.viewMatrix * modelMatrix);

    unsigned int slot = 0;
    for (unsigned int i = 0; i < MaterialTextureType::last; i++) {
        // Set uniform telling the shader if a texture of the type was provided
        s_data.gBufferShader->setInt(
                textureTypeToHasTextureUniformName(MaterialTextureType::Type(i)),
                mesh.getMaterial()->getTextures(MaterialTextureType::Type(i)).empty() ? 0 : 1);
        unsigned int indexOfTextureOfType = 0;
        for (const std::shared_ptr<Texture> &texture : mesh.getMaterial()->getTextures(MaterialTextureType::Type(i)))
        {
            texture->bind(slot);
            s_data.gBufferShader->setInt(textureTypeToUniformName(MaterialTextureType::Type(i)) + std::to_string(indexOfTextureOfType), slot);
            slot++, indexOfTextureOfType++;
        }
    }

    mesh.getVertexArray()->bind();
    GL::drawIndexed(mesh.getVertexArray());

    // Draw for shadow map
    s_data.shadowMapShader->bind();
    GL::viewport(s_data.shadowMapSize, s_data.shadowMapSize);
    for (ShadowMapLevel &shadowMap : s_data.shadowMap)
    {
        s_data.shadowMapShader->setMat4("u_lightSpaceModelMatrix", shadowMap.projectionView * modelMatrix);
        shadowMap.framebuffer->bind();
        GL::drawIndexed(mesh.getVertexArray());
    }
    GL::viewport(s_data.width, s_data.height);
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

// DEBUG
#ifdef DEBUG
std::vector<std::shared_ptr<Texture>> getShadowMaps()
{
    std::vector<std::shared_ptr<Texture>> shadowMaps;
    for (ShadowMapLevel shadowMap : s_data.shadowMap)
    {
        shadowMaps.emplace_back(shadowMap.framebuffer->getDepthAttachment());
    }
    return shadowMaps;
}
#endif // DEBUG

