#include "engine/ForwardRenderer.hpp"

#include "engine/Components.hpp"
#include "engine/assets/Material.hpp"

#include <glm/fwd.hpp>
#include <glm/gtx/string_cast.hpp>
#include <memory>
#include <opengl/gl.hpp>

#include <flecs.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <utility>

namespace engine {

constexpr size_t MAX_VERTICES_IN_LINES_BATCH = 2048;

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
    std::list<ShaderCompileTimeParameter> meshShaderParams;

    if (std::to_underlying(m_flags & Flags::enableShadowMapping))
    {
        meshShaderParams.emplace_back("USE_SHADOW_MAPPING");
    }
    if (std::to_underlying(m_flags & Flags::enableParallaxMapping))
    {
        meshShaderParams.emplace_back("USE_PARALLAX_MAPPING");
    }

    m_staticMeshShader = std::make_shared<Shader>(
        "assets/shaders/forwardStaticMeshVertex.glsl",
        "assets/shaders/forwardStaticMeshFragment.glsl",
        meshShaderParams);

    meshShaderParams.emplace_back("USE_SKINNING");
    m_animatedMeshShader = std::make_shared<Shader>(
        "assets/shaders/forwardStaticMeshVertex.glsl",
        "assets/shaders/forwardStaticMeshFragment.glsl",
        meshShaderParams);

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
        m_staticShadowMapShader = std::make_shared<Shader>(
            "assets/shaders/shadowMapVertex.glsl",
            "assets/shaders/shadowMapFragment.glsl");

        std::list<ShaderCompileTimeParameter> animatedMeshShadowMapShaderParameters {{"USE_SKINNING"}};
        m_animatedShadowMapShader = std::make_shared<Shader>(
            "assets/shaders/shadowMapVertex.glsl",
            "assets/shaders/shadowMapFragment.glsl",
            animatedMeshShadowMapShaderParameters);
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

    m_cubeLinesShader = std::make_shared<Shader>(
        "assets/shaders/renderCubeLinesVertex.glsl",
        "assets/shaders/renderCubeLinesFragment.glsl");
    m_cubeVertexArrayForLines = std::make_shared<VertexArray>();
    float cubeVerticesForLines[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
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

    m_linesBatchVertexArray = std::make_shared<VertexArray>();

    m_linesBatchVertexBuffer = std::make_shared<VertexBuffer>(MAX_VERTICES_IN_LINES_BATCH);
    m_linesBatchVertexBuffer->setLayout({ {ShaderDataType::float3, "a_position"} });
    m_linesBatchVertexArray->addVertexBuffer(m_linesBatchVertexBuffer);

    std::array<uint32_t, MAX_VERTICES_IN_LINES_BATCH> indices;
    for (uint32_t i = 0; i < indices.size(); i++)
    {
        indices[i] = i;
    }
    auto linesBatchIndexBufer = std::make_shared<IndexBuffer>(indices.data(), indices.size());
    m_linesBatchVertexArray->setIndexBuffer(linesBatchIndexBufer);
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
    float deltaTime, // in miliseconds
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const flecs::world &world,
    const AssetManager &assetManager,
    const FrameBuffer &renderTarget)
{
    if (deltaTime > 0.0f)
    {
        world.each([deltaTime, &assetManager](
            engine::components::AnimationPlayer &animationComponent)
        {
            const Animation *animation = assetManager.get(animationComponent.id);
            if (!animation) return;

            if (animationComponent.playing)
            {
                animationComponent.progress
                    = glm::mod(
                        animationComponent.progress + (deltaTime / 1000.0f),
                        animation->getDuration());
            }
        });
    }

    updateShadowMapLevels(cameraTransform, camera, world);
    // Draw for shadow map
    if (std::to_underlying(m_flags & Flags::enableShadowMapping))
    {
        world.each([&assetManager, this, &world](
            const components::StaticMesh &meshComponent,
            const components::Transform &transform)
        {
            drawMeshInShadowMaps(meshComponent, transform.getTransform(), assetManager, world);
        });
        world.each([&assetManager, this, &world](
            const components::AnimatedMesh &meshComponent,
            const components::AnimationPlayer *animationComponent,
            const components::Transform &transform)
        {
            drawMeshInShadowMaps(meshComponent, animationComponent, transform.getTransform(), assetManager, world);
        });
    }

    int nextFreeTextureSlotStaticMeshShader = 0;
    int nextFreeTextureSlotAnimatedMeshShader = 0;
    m_staticMeshShader->bind();
    setLightUniforms(*m_staticMeshShader, world, cameraTransform, nextFreeTextureSlotStaticMeshShader);
    m_animatedMeshShader->bind();
    setLightUniforms(*m_animatedMeshShader, world, cameraTransform, nextFreeTextureSlotAnimatedMeshShader);

    world.each([&assetManager, this, &camera, &cameraTransform, &renderTarget, nextFreeTextureSlotStaticMeshShader](
        const components::StaticMesh &meshComponent,
        const components::Transform &transform,
        const components::Material *materialComponent)
    {
        drawMesh(
            cameraTransform,
            camera,
            meshComponent,
            materialComponent,
            transform.getTransform(),
            assetManager,
            renderTarget,
            nextFreeTextureSlotStaticMeshShader);
        drawAABB(
            cameraTransform,
            camera,
            meshComponent,
            transform.getTransform(),
            assetManager,
            renderTarget);
    });
    world.each([&assetManager, this, &camera, &cameraTransform, &renderTarget, nextFreeTextureSlotAnimatedMeshShader](
        const components::AnimatedMesh &meshComponent,
        const components::AnimationPlayer *animationComponent,
        const components::Transform &transform,
        const components::Material *materialComponent)
    {
        drawMesh(
            cameraTransform,
            camera,
            meshComponent,
            materialComponent,
            animationComponent,
            transform.getTransform(),
            assetManager,
            renderTarget,
            nextFreeTextureSlotAnimatedMeshShader);
        drawSkeleton(
            cameraTransform,
            camera,
            meshComponent,
            animationComponent,
            transform.getTransform(),
            assetManager,
            renderTarget);
        //drawAABB(cameraTransform, camera, mesh->getAABB(), transform.getTransform(), renderTarget);
    });
    if (world.has<components::Skybox>())
    {
        const Cubemap *cubemap = assetManager.get(world.ensure<components::Skybox>().id);
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
    const engine::components::StaticMesh &meshComponent,
    const glm::mat4 &modelMatrix,
    const AssetManager &assetManager,
    const flecs::world &world) const
{
    const StaticMesh *mesh = assetManager.get(meshComponent.id);
    if (!mesh) {
        WARN("Static mesh with id: {} does not exist", std::to_underlying(meshComponent.id));
        return;
    }

    GL::setDepthTest(true);

    m_staticShadowMapShader->bind();
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
            m_staticShadowMapShader->setUniform(
                shadowMapComponent.levels[i].projectionView * modelMatrix,
                "u_lightSpaceModelMatrix");
            GL::drawIndexed(mesh->getVertexArray());
        }
    });
}

void ForwardRenderer::drawMeshInShadowMaps(
    const engine::components::AnimatedMesh &meshComponent,
    const engine::components::AnimationPlayer *animationComponent,
    const glm::mat4 &modelMatrix,
    const AssetManager &assetManager,
    const flecs::world &world) const
{
    const AnimatedMesh *mesh = assetManager.get(meshComponent.id);
    if (!mesh) {
        WARN("Animated mesh with id: {} does not exist", std::to_underlying(meshComponent.id));
        return;
    }
    const Animation *animation = animationComponent ? assetManager.get(animationComponent->id) : nullptr;
    if (!animation) {
        WARN("Animation with id: {} does not exist", std::to_underlying(animationComponent->id));
        return;
    }

    GL::setDepthTest(true);

    m_animatedShadowMapShader->bind();

    if (animationComponent && animation)
    {
        std::vector<glm::mat4> matrices
            = animation->getTransformations(
                animationComponent->progress,
                mesh->getSkeleton());

        for (BoneID boneId = 0; boneId < matrices.size(); boneId++)
        {
            m_animatedShadowMapShader->setUniform(matrices[boneId], "u_finalBonesMatrices[{}]", boneId);
        }
    }
    else
    {
        for (BoneID boneId = 0; boneId < mesh->getSkeleton().bones.size(); boneId++)
        {
            m_animatedShadowMapShader->setUniform(glm::mat4(1.0f), "u_finalBonesMatrices[{}]", boneId);
        }
    }

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
            m_animatedShadowMapShader->setUniform(
                shadowMapComponent.levels[i].projectionView * modelMatrix,
                "u_lightSpaceModelMatrix");
            GL::drawIndexed(mesh->getVertexArray());
        }
    });
}

// nextFreeSlot gets incremented for each bound texture
void ForwardRenderer::setMaterialUniforms(
    Shader &shader,
    const Material *material,
    const AssetManager &assetManager,
    int &nextFreeTextureSlot) const
{
    if (material)
    {
        for (MaterialTextureType::Type i : {
            MaterialTextureType::diffuse,
            MaterialTextureType::specular,
            MaterialTextureType::normal,
            MaterialTextureType::height
        }) {
            // Set uniform telling the shader if a texture of the type was provided
            shader.setUniform(
                    material->getTextureIds(i).empty() ? 0 : 1,
                    "{}",
                    textureTypeToHasTextureUniformName(i));
            unsigned int indexOfTextureOfType = 0;
            for (const TextureId &textureId : material->getTextureIds(i))
            {
                const Texture *texture = assetManager.get(textureId);
                if (texture)
                {
                    texture->bind(nextFreeTextureSlot);
                    shader.setUniform(
                        nextFreeTextureSlot,
                        "{}{}",
                        textureTypeToUniformName(i),
                        indexOfTextureOfType);
                    nextFreeTextureSlot++, indexOfTextureOfType++;
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
            shader.setUniform(0, "{}", textureTypeToHasTextureUniformName(i));
        }
    }
}

void ForwardRenderer::setLightUniforms(
    Shader &shader,
    const flecs::world &world,
    const engine::components::Transform &cameraTransform,
    int &nextFreeTextureSlot) const
{
    // Directional Light
    int lightIndex = 0;
    world.each([this, &nextFreeTextureSlot, &lightIndex, &cameraTransform, &shader](
        const components::DirectionalLight &light,
        const components::Transform &transform,
        components::DirectionalLightShadowMap *shadowMapComponent)
    {
        if (std::to_underlying(m_flags & Flags::enableShadowMapping) && shadowMapComponent)
        {
            shadowMapComponent->shadowMapAtlasFramebuffer.getDepthAttachment().bind(nextFreeTextureSlot);
            shader.setUniform(nextFreeTextureSlot, "u_directionalLightsShadowMapAtlas[{}]", lightIndex);
            nextFreeTextureSlot++;

            for (unsigned int i = 0; i < shadowMapComponent->levelCount; i++)
            {
                const components::DirectionalLightShadowMap::ShadowMapLevel &shadowMap
                    = shadowMapComponent->levels[i];
                glm::mat4 cameraSpaceToLightSpace
                    = shadowMap.projectionView * glm::inverse(cameraTransform.getView());
                shader.setUniform(
                    cameraSpaceToLightSpace,
                    "u_directionalLights[{}].shadowMapLevels[{}].cameraSpaceToLightSpace",
                    lightIndex, i);
                shader.setUniform(
                    shadowMap.cutoffDistance,
                    "u_directionalLights[{}].shadowMapLevels[{}].cutoffDistance",
                    lightIndex, i);
            }
            shader.setUniform(shadowMapComponent->levelCount, "u_directionalLights[{}].numOfShadowMapLevels", lightIndex);
        }

        glm::vec3 lightDirection = transform.getRotationMat3() * glm::vec3(0.0f, 0.0f, -1.0f);
        shader.setUniform(
            glm::mat3(cameraTransform.getView()) * lightDirection,
            "u_directionalLights[{}].directionInViewSpace", lightIndex);
        shader.setUniform(light.color, "u_directionalLights[{}].color", lightIndex);
        shader.setUniform(light.intensity, "u_directionalLights[{}].intensity", lightIndex);

        lightIndex++;
    });
    shader.setUniform(lightIndex, "u_numOfDirectionalLights");

    int i = 0;
    world.each([&i, &cameraTransform, &shader](
        const components::PointLight &light,
        const components::Transform & transform)
    {
        glm::vec3 lightPosInViewSpace = cameraTransform.getView() * glm::vec4(transform.position, 1.0f);

        shader.setUniform(lightPosInViewSpace, "u_pointLights[{}].posInViewSpace", i);
        shader.setUniform(light.color, "u_pointLights[{}].color", i);
        shader.setUniform(light.intensity, "u_pointLights[{}].intensity", i);
        shader.setUniform(light.attenuation, "u_pointLights[{}].attenuation", i);
        i++;
    });
    shader.setUniform(i, "u_numOfPointLights");
}

void ForwardRenderer::drawMesh(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const engine::components::StaticMesh &meshComponent,
    const engine::components::Material *materialComponent,
    const glm::mat4 &modelMatrix,
    const AssetManager &assetManager,
    const FrameBuffer &renderTarget,
    int nextFreeTextureSlot) const
{
    const StaticMesh *mesh = assetManager.get(meshComponent.id);
    const Material *material = materialComponent ? assetManager.get(materialComponent->id) : nullptr;
    if (!mesh) {
        WARN("Static mesh with id: {} does not exist", std::to_underlying(meshComponent.id));
        return;
    }

    renderTarget.bind();
    m_staticMeshShader->bind();
    m_staticMeshShader->setUniform(camera.getProjection(), "u_projectionMatrix");
    m_staticMeshShader->setUniform(cameraTransform.getView() * modelMatrix, "u_viewModelMatrix");
    m_staticMeshShader->setUniform(0.05f, "u_parallaxScale");
    GL::setDepthTest(true);

    setMaterialUniforms(*m_staticMeshShader, material, assetManager, nextFreeTextureSlot);

    mesh->getVertexArray().bind();
    GL::viewport(renderTarget.getWidth(), renderTarget.getHeight());
    GL::drawIndexed(mesh->getVertexArray());
}

void ForwardRenderer::drawMesh(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const engine::components::AnimatedMesh &meshComponent,
    const engine::components::Material *materialComponent,
    const engine::components::AnimationPlayer *animationComponent,
    const glm::mat4 &modelMatrix,
    const AssetManager &assetManager,
    const FrameBuffer &renderTarget,
    int nextFreeTextureSlot) const
{
    const AnimatedMesh *mesh = assetManager.get(meshComponent.id);
    const Material *material = materialComponent ? assetManager.get(materialComponent->id) : nullptr;
    const Animation *animation = animationComponent ? assetManager.get(animationComponent->id) : nullptr;
    if (!mesh) {
        WARN("Animated mesh with id: {} does not exist", std::to_underlying(meshComponent.id));
        return;
    }

    renderTarget.bind();
    m_animatedMeshShader->bind();
    m_animatedMeshShader->setUniform(camera.getProjection(), "u_projectionMatrix");
    m_animatedMeshShader->setUniform(cameraTransform.getView() * modelMatrix, "u_viewModelMatrix");
    m_animatedMeshShader->setUniform(0.05f, "u_parallaxScale");
    GL::setDepthTest(true);

    if (animationComponent && animation)
    {
        std::vector<glm::mat4> matrices
            = animation->getTransformations(
                animationComponent->progress,
                mesh->getSkeleton());

        for (BoneID boneId = 0; boneId < matrices.size(); boneId++)
        {
            m_animatedMeshShader->setUniform(matrices[boneId], "u_finalBonesMatrices[{}]", boneId);
        }
    }
    else
    {
        for (BoneID boneId = 0; boneId < mesh->getSkeleton().bones.size(); boneId++)
        {
            m_animatedMeshShader->setUniform(glm::mat4(1.0f), "u_finalBonesMatrices[{}]", boneId);
        }
    }

    setMaterialUniforms(*m_animatedMeshShader, material, assetManager, nextFreeTextureSlot);

    mesh->getVertexArray().bind();
    GL::viewport(renderTarget.getWidth(), renderTarget.getHeight());
    GL::drawIndexed(mesh->getVertexArray());
}

void ForwardRenderer::drawAABB(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const engine::components::StaticMesh &meshComponent,
    const glm::mat4 &modelMatrix,
    const AssetManager &assetManager,
    const FrameBuffer &renderTarget) const
{
    const StaticMesh *mesh = assetManager.get(meshComponent.id);
    if (!mesh) {
        WARN("Static mesh with id: {} does not exist", std::to_underlying(meshComponent.id));
        return;
    }

    const AxisAlignedBoundingBox &aabb = mesh->getAABB();
    renderTarget.bind();
    m_cubeLinesShader->bind();
    glm::mat4 matrix
        = camera.getProjection() * cameraTransform.getView()
        * modelMatrix
        * glm::translate(glm::mat4(1.0f), aabb.min)
        * glm::scale(glm::mat4(1.0f), aabb.max - aabb.min);
    m_cubeLinesShader->setUniform(matrix, "u_projectionView");
    GL::viewport(renderTarget.getWidth(), renderTarget.getHeight());
    GL::drawLines(*m_cubeVertexArrayForLines);
}

glm::vec3 toVec3(const glm::vec4 &v)
{
    return {v.x/v.w, v.y/v.w, v.z/v.w};
}

void ForwardRenderer::drawSkeleton(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const engine::components::AnimatedMesh &meshComponent,
    const engine::components::AnimationPlayer *animationComponent,
    const glm::mat4 &modelMatrix,
    const AssetManager &assetManager,
    const FrameBuffer &renderTarget) const
{
    const AnimatedMesh *mesh = assetManager.get(meshComponent.id);
    if (!mesh) {
        WARN("Animated mesh with id: {} does not exist", std::to_underlying(meshComponent.id));
        return;
    }

    const Animation *animation = animationComponent ? assetManager.get(animationComponent->id) : nullptr;
    const Skeleton &skeleton = mesh->getSkeleton();

    renderTarget.bind();
    m_cubeLinesShader->bind();
    glm::mat4 matrix
        = camera.getProjection() * cameraTransform.getView()
        * modelMatrix;
    m_cubeLinesShader->setUniform(matrix, "u_projectionView");

    GL::viewport(renderTarget.getWidth(), renderTarget.getHeight());
    GL::setDepthTestFunction(GL::DepthTestFunction::always);
    m_linesBatchVertices.clear();

    std::vector<glm::mat4> animated
        = animation->getTransformations(animationComponent->progress, skeleton);
    for (size_t i = 1; i < skeleton.bones.size(); i++)
    {
        m_linesBatchVertices.emplace_back(
            toVec3(
                (
                    animated[i]
                    * glm::inverse(skeleton.bones[i].offsetMatrix)
                )[3]
            )
        );
        m_linesBatchVertices.emplace_back(
            toVec3(
                (
                    animated[skeleton.bones[i].parentID]
                    * glm::inverse(skeleton.bones[skeleton.bones[i].parentID].offsetMatrix)
                )[3]
            )
        );
    }
    m_linesBatchVertexBuffer->setData(
        m_linesBatchVertices.data(),
        m_linesBatchVertices.size() * sizeof(m_linesBatchVertices[0]));
    GL::drawLines(*m_linesBatchVertexArray, m_linesBatchVertices.size());
    GL::setDepthTestFunction(GL::DepthTestFunction::less);
}

void ForwardRenderer::drawSkybox(
    const engine::components::Transform &cameraTransform,
    const engine::components::Camera &camera,
    const Cubemap &cubemap,
    const FrameBuffer &renderTarget) const
{
    renderTarget.bind();
    GL::setDepthTest(true);
    GL::setDepthTestFunction(GL::DepthTestFunction::lessEqual);
    m_skyboxShader->bind();
    m_skyboxShader->setUniform(
        camera.getProjection() * glm::mat4(glm::mat3(cameraTransform.getView())),
        "u_projectionView");
    cubemap.bind(0);
    m_skyboxShader->setUniform(0, "u_skybox");
    GL::viewport(renderTarget.getWidth(), renderTarget.getHeight());
    GL::drawIndexed(*m_cubeVertexArray);
    GL::setDepthTestFunction(GL::DepthTestFunction::less);
}

} // namespace engine
