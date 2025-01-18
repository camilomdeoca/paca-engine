#include "engine/ForwardRenderer.hpp"

#include "engine/Components.hpp"
#include "engine/assets/Material.hpp"

#include <glm/fwd.hpp>
#include <opengl/gl.hpp>

#include <flecs.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <utility>

namespace engine {

ForwardRenderer::ForwardRenderer()
{}

ForwardRenderer::ForwardRenderer(Flags flags)
{
    init(flags);
}

ForwardRenderer::~ForwardRenderer()
{}

void ForwardRenderer::init(Flags flags)
{
    m_flags = flags;
    std::list<ShaderCompileTimeParameter> staticMeshShaderParams;

    if (std::to_underlying(m_flags & Flags::enableShadowMapping))
    {
        staticMeshShaderParams.emplace_back("USE_SHADOW_MAPPING");
    }
    if (std::to_underlying(m_flags & Flags::enableParallaxMapping))
    {
        staticMeshShaderParams.emplace_back("USE_PARALLAX_MAPPING");
    }

    m_staticMeshShader = std::make_shared<Shader>(
        "assets/shaders/forwardStaticMeshVertex.glsl",
        "assets/shaders/forwardStaticMeshFragment.glsl",
        staticMeshShaderParams);

    if (std::to_underlying(m_flags & Flags::enableShadowMapping))
    {
        //m_shadowMapSize = m_parameters.shadowMapSize;

        //float prevSplit = 0.0f;
        //m_shadowMapLevels.resize(m_parameters.viewFrustumSplits.size());
        //INFO("Shadow map levels limits:");
        //for (unsigned int i = 0; i < m_parameters.viewFrustumSplits.size(); i++)
        //{
        //    m_shadowMapLevels[i].near = prevSplit;
        //    m_shadowMapLevels[i].far = m_parameters.viewFrustumSplits[i];
        //    prevSplit = m_parameters.viewFrustumSplits[i];
        //    m_shadowMapLevels[i].cutoffDistance = m_parameters.viewFrustumSplits[i];
        //    INFO("\t Level {}: {} {}", i, m_shadowMapLevels[i].near, m_shadowMapLevels[i].far);
        //}

        //FrameBufferParameters shadowDepthMapBufferParams;
        //shadowDepthMapBufferParams.width = m_shadowMapSize;
        //shadowDepthMapBufferParams.height = m_shadowMapSize * m_shadowMapLevels.size();
        //shadowDepthMapBufferParams.textureAttachmentFormats = { Texture::Format::depth24 };
        //m_shadowMapAtlasFramebuffer = std::make_shared<FrameBuffer>(shadowDepthMapBufferParams);
        //m_shadowMapAtlasFramebuffer->getDepthAttachment()->setBorderColor({1.0f, 1.0f, 1.0f, 1.0f});
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

    m_cubeLinesShader = std::make_shared<Shader>("assets/shaders/renderCubeLinesVertex.glsl", "assets/shaders/renderCubeLinesFragment.glsl");
    m_cubeVertexArrayForLines = std::make_shared<VertexArray>();
    float cubeVerticesForLines[] = {
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };
    uint32_t cubeIndicesForLines[] = {
        // Front face
        0, 1, 1, 2, 2, 3, 3, 0,

        // Conections to back face
        0, 4, 1, 5, 2, 6, 3, 7,

        // Back face
        4, 5, 5, 6, 6, 7, 7, 4,
    };

    std::shared_ptr<VertexBuffer> cubeVertexBufferForLines
        = std::make_shared<VertexBuffer>(
            reinterpret_cast<uint8_t*>(cubeVerticesForLines),
            sizeof(cubeVerticesForLines)
        );
    cubeVertexBufferForLines->setLayout({ {ShaderDataType::float3, "a_position"} });
    m_cubeVertexArrayForLines->addVertexBuffer(cubeVertexBufferForLines);

    std::shared_ptr<IndexBuffer> cubeIndexBufferForLines
        = std::make_shared<IndexBuffer>(
            cubeIndicesForLines,
            sizeof(cubeIndicesForLines) / sizeof(uint32_t)
        );
    m_cubeVertexArrayForLines->setIndexBuffer(cubeIndexBufferForLines);
}

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

void ForwardRenderer::updateShadowMapLevels(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const flecs::world &world)
{
    // Calculate view and projection (if the camera's projection changed) for redering each shadowMap
    if (!std::to_underlying(m_flags & Flags::enableShadowMapping))
        return;

    world.each([&camera, &cameraTransform](
        const components::DirectionalLight &light,
        const components::Transform &transform,
        components::DirectionalLightShadowMap &shadowMapComponent)
    {
        static glm::mat4 previousProjectionMatrix = glm::mat4(0.0f);
        bool projectionChanged = previousProjectionMatrix != camera.getProjection();
        previousProjectionMatrix = camera.getProjection();

        // Clear the shadowMap atlas
        shadowMapComponent.shadowMapAtlasFramebuffer.bind();
        GL::clear();

        for (unsigned int i = 0; i < shadowMapComponent.levelCount; i++)
        {
            components::DirectionalLightShadowMap::ShadowMapLevel &shadowMap
                = shadowMapComponent.levels[i];

            // If the camera's projection changed the frustum that the shadowMap has to contain
            // changed so we need to recalculate it
            if (projectionChanged)
            {
                shadowMap.frustumProjectionMatrix = glm::perspective(
                        glm::radians(camera.fov), 
                        camera.aspect, 
                        std::max(shadowMap.near, camera.near), 
                        std::min(shadowMap.far, camera.far));
            }

            std::array<glm::vec3, 8> corners
                = getFrustumCorners(
                    shadowMap.frustumProjectionMatrix
                    * cameraTransform.getView());
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

            glm::vec3 lightDirection = transform.getRotationMat3() * glm::vec3(0.0f, 0.0f, -1.0f);
            glm::mat3 lightRotMatrix = glm::mat3(glm::lookAt(-lightDirection, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}));
            const float pixelSize = shadowMap.maxDiagonal / shadowMapComponent.shadowMapSize;

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
    });
}

void ForwardRenderer::renderWorld(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const flecs::world &world,
    const AssetManager &assetManager,
    const FrameBuffer &renderTarget)
{
    updateShadowMapLevels(cameraTransform, camera, world);
    // Draw for shadow map
    if (std::to_underlying(m_flags & Flags::enableShadowMapping))
    {
        world.each([&assetManager, this, &world](const components::StaticMesh &meshComponent, const components::Transform &transform) {
            const StaticMesh *mesh = assetManager.get(meshComponent.id);
            if (mesh) drawMeshInShadowMaps(*mesh, transform.getTransform(), world);
        });
        //world.each([&resourceManager, this, &world](const components::AnimatedMesh &meshComponent, const components::Transform &transform) {
        //    const AnimatedMesh &mesh = resourceManager.getAnimatedMesh(meshComponent.id);
        //    drawMeshInShadowMaps(mesh, transform, world);
        //});
    }

    world.each([&assetManager, this, &world, &camera, &cameraTransform, &renderTarget](
        const components::StaticMesh &meshComponent,
        const components::Transform &transform,
        const components::Material &materialComponent)
    {
        const StaticMesh *mesh = assetManager.get(meshComponent.id);
        const Material *material = assetManager.get(materialComponent.id);
        if (mesh)
        {
            drawMesh(cameraTransform, camera, *mesh, material, transform.getTransform(), world, assetManager, renderTarget);
            drawAABB(cameraTransform, camera, mesh->getAABB(), transform.getTransform(), renderTarget);
        }
    });
    //world.each([&resourceManager, this, &world, &camera](
    //    const components::AnimatedMesh &meshComponent,
    //    const components::Transform &transform,
    //    const components::Material &materialComponent)
    //{
    //    const AnimatedMesh &mesh = resourceManager.getAnimatedMesh(meshComponent.id);
    //    const Material &material = resourceManager.getMaterial(materialComponent.id);
    //    drawMesh(camera, mesh, material, transform, world, resourceManager);
    //});
    if (world.has<components::Skybox>())
    {
        const Texture *cubemap = assetManager.get(world.ensure<components::Skybox>().id);
        if (cubemap) drawSkybox(cameraTransform, camera, *cubemap, renderTarget);
    }
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
    const StaticMesh &mesh,
    const glm::mat4 &modelMatrix,
    const flecs::world &world) const
{
    GL::setDepthTest(true);

    m_shadowMapShader->bind();
    world.each([this, &modelMatrix, &mesh](
        const components::DirectionalLight &light,
        const components::Transform &transform,
        components::DirectionalLightShadowMap &shadowMapComponent)
    {
        shadowMapComponent.shadowMapAtlasFramebuffer.bind();
        for (unsigned int i = 0; i < shadowMapComponent.levelCount; i++)
        {
            GL::viewport(
                0,
                i*shadowMapComponent.shadowMapSize,
                shadowMapComponent.shadowMapSize,
                shadowMapComponent.shadowMapSize);
            m_shadowMapShader->setMat4(
                "u_lightSpaceModelMatrix",
                shadowMapComponent.levels[i].projectionView * modelMatrix);
            GL::drawIndexed(mesh.getVertexArray());
        }
    });
}

void ForwardRenderer::drawMeshInShadowMaps(
    const AnimatedMesh &mesh,
    const glm::mat4 &modelMatrix,
    const flecs::world &world) const
{
    GL::setDepthTest(true);

    m_shadowMapShader->bind();
    world.each([this, &modelMatrix, &mesh](
        const components::DirectionalLight &light,
        const components::Transform &transform,
        components::DirectionalLightShadowMap &shadowMapComponent)
    {
        shadowMapComponent.shadowMapAtlasFramebuffer.bind();
        for (unsigned int i = 0; i < shadowMapComponent.levelCount; i++)
        {
            GL::viewport(
                0,
                i*shadowMapComponent.shadowMapSize,
                shadowMapComponent.shadowMapSize,
                shadowMapComponent.shadowMapSize);
            m_shadowMapShader->setMat4(
                "u_lightSpaceModelMatrix",
                shadowMapComponent.levels[i].projectionView * modelMatrix);
            GL::drawIndexed(mesh.getVertexArray());
        }
    });
}

void ForwardRenderer::drawMesh(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const StaticMesh &mesh,
    const Material *material,
    const glm::mat4 &modelMatrix,
    const flecs::world &world,
    const AssetManager &assetManager,
    const FrameBuffer &renderTarget) const
{
    renderTarget.bind();
    m_staticMeshShader->bind();
    m_staticMeshShader->setMat4("u_projectionMatrix", camera.getProjection());
    m_staticMeshShader->setMat4("u_viewModelMatrix", cameraTransform.getView() * modelMatrix);
    m_staticMeshShader->setFloat("u_parallaxScale", 0.05f);
    GL::setDepthTest(true);


    unsigned int slot = 0;
    if (material)
    {
        for (MaterialTextureType::Type i : {
            MaterialTextureType::diffuse,
            MaterialTextureType::specular,
            MaterialTextureType::normal,
            MaterialTextureType::height
        }) {
            // Set uniform telling the shader if a texture of the type was provided
            m_staticMeshShader->setInt(
                    textureTypeToHasTextureUniformName(i),
                    material->getTextureIds(i).empty() ? 0 : 1);
            unsigned int indexOfTextureOfType = 0;
            for (const TextureId &textureId : material->getTextureIds(i))
            {
                const Texture *texture = assetManager.get(textureId);
                if (texture)
                {
                    texture->bind(slot);
                    m_staticMeshShader->setInt(textureTypeToUniformName(i) + std::to_string(indexOfTextureOfType), slot);
                    slot++, indexOfTextureOfType++;
                }
            }
        }
    }
    else
    {
        // When the mesh does not have a material set every has_{texture_type} uniform to false
        for (MaterialTextureType::Type i : {
            MaterialTextureType::diffuse,
            MaterialTextureType::specular,
            MaterialTextureType::normal,
            MaterialTextureType::height
        }) {
            m_staticMeshShader->setInt(textureTypeToHasTextureUniformName(i), 0);
        }
    }

    // Directional Light
    int lightIndex = 0;
    world.each([this, &slot, &lightIndex, &cameraTransform](
        const components::DirectionalLight &light,
        const components::Transform &transform,
        components::DirectionalLightShadowMap *shadowMapComponent)
    {
        if (std::to_underlying(m_flags & Flags::enableShadowMapping) && shadowMapComponent)
        {
            shadowMapComponent->shadowMapAtlasFramebuffer.getDepthAttachment().bind(slot);
            m_staticMeshShader->setInt(std::format("u_directionalLightsShadowMapAtlas[{}]", lightIndex), slot);
            slot++;

            for (unsigned int i = 0; i < shadowMapComponent->levelCount; i++)
            {
                const components::DirectionalLightShadowMap::ShadowMapLevel &shadowMap
                    = shadowMapComponent->levels[i];
                glm::mat4 cameraSpaceToLightSpace = shadowMap.projectionView * glm::inverse(cameraTransform.getView());
                m_staticMeshShader->setMat4(std::format("u_directionalLights[{}].shadowMapLevels[{}].cameraSpaceToLightSpace", lightIndex, i), cameraSpaceToLightSpace);
                m_staticMeshShader->setFloat(std::format("u_directionalLights[{}].shadowMapLevels[{}].cutoffDistance", lightIndex, i), shadowMap.cutoffDistance);
            }
            m_staticMeshShader->setInt(std::format("u_directionalLights[{}].numOfShadowMapLevels", lightIndex), shadowMapComponent->levelCount);
        }

        glm::vec3 lightDirection = transform.getRotationMat3() * glm::vec3(0.0f, 0.0f, -1.0f);
        m_staticMeshShader->setFloat3(std::format("u_directionalLights[{}].directionInViewSpace", lightIndex), glm::mat3(cameraTransform.getView()) * lightDirection);
        m_staticMeshShader->setFloat3(std::format("u_directionalLights[{}].color", lightIndex), light.color);
        m_staticMeshShader->setFloat(std::format("u_directionalLights[{}].intensity", lightIndex), light.intensity);

        lightIndex++;
    });
    m_staticMeshShader->setInt("u_numOfDirectionalLights", lightIndex);

    int i = 0;
    world.each([ this, &i, &cameraTransform](
        const components::PointLight &light,
        const components::Transform & transform)
    {
        glm::vec3 lightPosInViewSpace = cameraTransform.getView() * glm::vec4(transform.position, 1.0f);

        m_staticMeshShader->setFloat3(std::format("u_pointLights[{}].posInViewSpace", i), lightPosInViewSpace);
        m_staticMeshShader->setFloat3(std::format("u_pointLights[{}].color", i), light.color);
        m_staticMeshShader->setFloat(std::format("u_pointLights[{}].intensity", i), light.intensity);
        m_staticMeshShader->setFloat(std::format("u_pointLights[{}].attenuation", i), light.attenuation);
        i++;
    });
    m_staticMeshShader->setInt("u_numOfPointLights", i);

    mesh.getVertexArray().bind();
    GL::viewport(renderTarget.getWidth(), renderTarget.getHeight());
    GL::drawIndexed(mesh.getVertexArray());
}

void ForwardRenderer::drawAABB(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const AxisAlignedBoundingBox &aabb,
    const glm::mat4 &modelMatrix,
    const FrameBuffer &renderTarget) const
{
    renderTarget.bind();
    m_cubeLinesShader->bind();
    glm::mat4 matrix
        = camera.getProjection() * cameraTransform.getView()
        * modelMatrix
        * glm::translate(glm::mat4(1.0f), aabb.min)
        * glm::scale(glm::mat4(1.0f), aabb.max - aabb.min);
    m_cubeLinesShader->setMat4("u_projectionView", matrix);
    GL::viewport(renderTarget.getWidth(), renderTarget.getHeight());
    GL::drawLines(*m_cubeVertexArrayForLines);
}

void ForwardRenderer::drawSkybox(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const Texture &cubemap,
    const FrameBuffer &renderTarget) const
{
    renderTarget.bind();
    GL::setDepthTest(true);
    GL::setDepthTestFunction(GL::DepthTestFunction::lessEqual);
    m_skyboxShader->bind();
    m_skyboxShader->setMat4("u_projectionView", camera.getProjection() * glm::mat4(glm::mat3(cameraTransform.getView())));
    cubemap.bind(0);
    m_skyboxShader->setInt("u_skybox", 0);
    GL::viewport(renderTarget.getWidth(), renderTarget.getHeight());
    GL::drawIndexed(*m_cubeVertexArray);
    GL::setDepthTestFunction(GL::DepthTestFunction::less);
}

} // namespace engine
