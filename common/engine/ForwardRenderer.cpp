#include "engine/ForwardRenderer.hpp"

#include "engine/Material.hpp"
#include "opengl/gl.hpp"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

ForwardRenderer::ForwardRenderer()
{}

ForwardRenderer::ForwardRenderer(const Parameters &parameters)
{
    init(parameters);
}

ForwardRenderer::~ForwardRenderer()
{}

void ForwardRenderer::init(const Parameters &parameters)
{
    m_parameters = parameters;
    std::list<ShaderCompileTimeParameter> staticMeshShaderParams;

    if (m_parameters.flags & Parameters::enableShadowMapping)
    {
        staticMeshShaderParams.emplace_back("USE_SHADOW_MAPPING");
    }

    m_staticMeshShader = std::make_shared<Shader>(
        "assets/shaders/forwardStaticMeshVertex.glsl",
        "assets/shaders/forwardStaticMeshFragment.glsl",
        staticMeshShaderParams);

    if (m_parameters.flags & Parameters::enableShadowMapping)
    {
        m_shadowMapSize = m_parameters.shadowMapSize;

        float prevSplit = 0.0f;
        m_shadowMapLevels.resize(m_parameters.viewFrustumSplits.size());
        INFO("Shadow map levels limits:");
        for (unsigned int i = 0; i < m_parameters.viewFrustumSplits.size(); i++)
        {
            m_shadowMapLevels[i].near = prevSplit;
            m_shadowMapLevels[i].far = m_parameters.viewFrustumSplits[i];
            prevSplit = m_parameters.viewFrustumSplits[i];
            m_shadowMapLevels[i].cutoffDistance = m_parameters.viewFrustumSplits[i];
            INFO("\t Level {}: {} {}", i, m_shadowMapLevels[i].near, m_shadowMapLevels[i].far);
        }

        FrameBufferParameters shadowDepthMapBufferParams;
        shadowDepthMapBufferParams.width = m_shadowMapSize;
        shadowDepthMapBufferParams.height = m_shadowMapSize;
        shadowDepthMapBufferParams.textureAttachmentFormats = { Texture::Format::depth24 };
        for (ShadowMapLevel &shadowMapLevel : m_shadowMapLevels)
        {
            shadowMapLevel.framebuffer = std::make_shared<FrameBuffer>(shadowDepthMapBufferParams);
            shadowMapLevel.framebuffer->getDepthAttachment()->setBorderColor({1.0f, 1.0f, 1.0f, 1.0f});
        }
        m_shadowMapShader = std::make_shared<Shader>("assets/shaders/shadowMapVertex.glsl", "assets/shaders/shadowMapFragment.glsl");
    }

    m_skyboxShader = std::make_shared<Shader>("assets/shaders/skyboxVertex.glsl", "assets/shaders/skyboxFragment.glsl");

    m_cubeVertexArray = std::make_shared<VertexArray>();
    float cubeVertices[] = {
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f
    };
    uint32_t cubeIndices[] = {
        // Front face (inverted)
        2, 1, 0,
        3, 1, 2,

        // Back face (inverted)
        5, 6, 4,
        7, 6, 5,

        // Left face (inverted)
        4, 2, 0,
        6, 2, 4,

        // Right face (inverted)
        3, 5, 1,
        7, 5, 3,

        // Top face (inverted)
        6, 3, 2,
        7, 3, 6,

        // Bottom face (inverted)
        1, 4, 0,
        5, 4, 1
    };

    std::shared_ptr<VertexBuffer> cubeVertexBuffer
        = std::make_shared<VertexBuffer>(
            reinterpret_cast<uint8_t*>(cubeVertices),
            sizeof(cubeVertices)
        );
    cubeVertexBuffer->setLayout({ {ShaderDataType::float3, "a_position"} });
    m_cubeVertexArray->addVertexBuffer(cubeVertexBuffer);

    std::shared_ptr<IndexBuffer> cubeIndexBuffer
        = std::make_shared<IndexBuffer>(
            cubeIndices,
            sizeof(cubeIndices) / sizeof(uint32_t)
        );
    m_cubeVertexArray->setIndexBuffer(cubeIndexBuffer);
}

void ForwardRenderer::resize(uint32_t width, uint32_t height)
{}

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

void ForwardRenderer::updateShadowMapLevels(const PerspectiveCamera &camera, const World &world)
{
    // Calculate view and projection (if the camera's projection changed) for redering each shadowMap
    if (m_parameters.flags & Parameters::enableShadowMapping)
    {
        float near = camera.getNear(), far = camera.getFar();
        static glm::mat4 previousProjectionMatrix = glm::mat4(0.0f);
        bool projectionChanged = previousProjectionMatrix != camera.getProjectionMatrix();
        previousProjectionMatrix = camera.getProjectionMatrix();
        for (ShadowMapLevel &shadowMap : m_shadowMapLevels)
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

            std::array<glm::vec3, 8> corners = getFrustumCorners(shadowMap.frustumProjectionMatrix * camera.getViewMatrix());
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

            const glm::vec3 &lightDirection = world.getDirectionalLights()[0]->getDirection();
            glm::mat3 lightRotMatrix = glm::mat3(glm::lookAt(-lightDirection, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
            const float pixelSize = shadowMap.maxDiagonal / m_parameters.shadowMapSize;

            // Align coordinate axis to lightView so when the position of the shadowMap is rounded
            // It is rounded aligned with the pixels
            center = lightRotMatrix * center;
            center.x = std::round(center.x / pixelSize) * pixelSize;
            center.y = std::round(center.y / pixelSize) * pixelSize;
            center = glm::inverse(lightRotMatrix) * center;
            //center.z = std::round(center.z / pixelSize) * pixelSize;

            float &side = shadowMap.maxDiagonal;
            float depthOfShadowMap = 5.0f; // Tune this parameter
            shadowMap.projectionView = glm::ortho(-side/2, side/2, -side/2, side/2, -side*depthOfShadowMap, side*depthOfShadowMap)
                    * glm::lookAt(center - lightDirection, center, glm::vec3(0.0f, 1.0f, 0.0f));
        }
    }
}

void ForwardRenderer::renderWorld(const PerspectiveCamera &camera, const World &world)
{
    updateShadowMapLevels(camera, world);
    // Draw for shadow map
    if (m_parameters.flags & Parameters::enableShadowMapping)
    {
        for (const std::shared_ptr<Model> &model : world.getModels())
        {
            drawModelInShadowMaps(camera, *model, world);
        }
    }


    for (const std::shared_ptr<Model> &model : world.getModels())
    {
        drawModel(camera, *model, world);
    }
    if (world.getSkyboxTexture())
        drawSkybox(camera, *world.getSkyboxTexture());
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

void ForwardRenderer::drawMeshInShadowMaps(
    const Mesh &mesh,
    const glm::mat4 &modelMatrix,
    const World &world) const
{
    GL::setDepthTest(true);

    m_shadowMapShader->bind();
    GL::viewport(m_parameters.shadowMapSize, m_parameters.shadowMapSize);
    for (const ShadowMapLevel &shadowMap : m_shadowMapLevels)
    {
        m_shadowMapShader->setMat4("u_lightSpaceModelMatrix", shadowMap.projectionView * modelMatrix);
        shadowMap.framebuffer->bind();
        GL::drawIndexed(mesh.getVertexArray());
    }
    GL::viewport(m_parameters.width, m_parameters.height);
}

void ForwardRenderer::drawMesh(
    const PerspectiveCamera &camera,
    const Mesh &mesh,
    const glm::mat4 &modelMatrix,
    const World &world) const
{
    FrameBuffer::unbind();
    m_staticMeshShader->bind();
    m_staticMeshShader->setMat4("u_projectionMatrix", camera.getProjectionMatrix());
    m_staticMeshShader->setMat4("u_viewModelMatrix", camera.getViewMatrix() * modelMatrix);
    m_staticMeshShader->setFloat("u_parallaxScale", 0.05f);
    GL::setDepthTest(true);

    unsigned int slot = 0;
    for (MaterialTextureType::Type i : {
        MaterialTextureType::diffuse,
        MaterialTextureType::specular,
        MaterialTextureType::normal
    }) {
        // Set uniform telling the shader if a texture of the type was provided
        m_staticMeshShader->setInt(
                textureTypeToHasTextureUniformName(i),
                mesh.getMaterial()->getTextures(i).empty() ? 0 : 1);
        unsigned int indexOfTextureOfType = 0;
        for (const std::shared_ptr<Texture> &texture : mesh.getMaterial()->getTextures(i))
        {
            texture->bind(slot);
            m_staticMeshShader->setInt(textureTypeToUniformName(i) + std::to_string(indexOfTextureOfType), slot);
            slot++, indexOfTextureOfType++;
        }
    }

    // Directional Light
    if (world.getDirectionalLights().size() > 0) {
        if (m_parameters.flags & Parameters::enableShadowMapping)
        {
            for (unsigned int i = 0; i < m_shadowMapLevels.size(); i++)
            {
                const ShadowMapLevel &shadowMap = m_shadowMapLevels[i];
                shadowMap.framebuffer->getDepthAttachment()->bind(3 + i);
                m_staticMeshShader->setInt("u_shadowMaps[" + std::to_string(i) + "].texture", 3 + i);
                glm::mat4 cameraSpaceToLightSpace = shadowMap.projectionView * glm::inverse(camera.getViewMatrix());
                m_staticMeshShader->setMat4("u_shadowMaps[" + std::to_string(i) + "].cameraSpaceToLightSpace", cameraSpaceToLightSpace);
                m_staticMeshShader->setFloat("u_shadowMaps[" + std::to_string(i) + "].cutoffDistance", shadowMap.cutoffDistance);
            }
            m_staticMeshShader->setInt("u_numOfShadowMapLevels", m_shadowMapLevels.size());
        }

        const std::shared_ptr<DirectionalLight> &directionalLight = world.getDirectionalLights()[0];
        m_staticMeshShader->setFloat3("u_directionalLight.directionInViewSpace", glm::mat3(camera.getViewMatrix()) * directionalLight->getDirection());
        m_staticMeshShader->setFloat3("u_directionalLight.color", directionalLight->getColor());
        m_staticMeshShader->setFloat("u_directionalLight.intensity", directionalLight->getIntensity());
    }

    for (unsigned int i = 0; i < world.getPointLights().size() && i < 10; i++)
    {
        const std::shared_ptr<PointLight> &light = world.getPointLights()[i];

        glm::vec3 lightPosInViewSpace
            = camera.getViewMatrix()
            * glm::vec4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 1.0f);

        m_staticMeshShader->setFloat3(std::format("u_lights[{}].posInViewSpace", i), lightPosInViewSpace);
        m_staticMeshShader->setFloat3(std::format("u_lights[{}].color", i), light->getColor());
        m_staticMeshShader->setFloat(std::format("u_lights[{}].intensity", i), light->getIntensity());
        m_staticMeshShader->setFloat(std::format("u_lights[{}].attenuation", i), light->getAttenuation());
    }
    m_staticMeshShader->setInt("u_numOfLights", world.getPointLights().size());

    mesh.getVertexArray()->bind();
    GL::drawIndexed(mesh.getVertexArray());
}

void ForwardRenderer::drawModelInShadowMaps(
    const PerspectiveCamera &camera,
    const Model &model,
    const World &world) const
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, model.getPosition());
    transform = glm::scale(transform, model.getScale());
    glm::quat rot(glm::radians(model.getRotation()));
    transform = transform * glm::mat4_cast(rot);

    for (const std::shared_ptr<Mesh> &mesh : model.getMeshes())
    {
        drawMeshInShadowMaps(*mesh, transform, world);
    }
}

void ForwardRenderer::drawModel(
    const PerspectiveCamera &camera,
    const Model &model,
    const World &world) const
{
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, model.getPosition());
    transform = glm::scale(transform, model.getScale());
    glm::quat rot(glm::radians(model.getRotation()));
    transform = transform * glm::mat4_cast(rot);

    for (const std::shared_ptr<Mesh> &mesh : model.getMeshes())
    {
        drawMesh(camera, *mesh, transform, world);
    }
}

void ForwardRenderer::drawSkybox(const PerspectiveCamera &camera, const Texture &cubemap) const
{
    GL::setDepthTest(true);
    GL::setDepthTestFunction(GL::DepthTestFunction::lessEqual);
    m_skyboxShader->bind();
    m_skyboxShader->setMat4("u_projectionView", camera.getProjectionMatrix() * glm::mat4(glm::mat3(camera.getViewMatrix())));
    cubemap.bind(0);
    m_skyboxShader->setInt("u_skybox", 0);
    GL::drawIndexed(m_cubeVertexArray);
    GL::setDepthTestFunction(GL::DepthTestFunction::less);
}
