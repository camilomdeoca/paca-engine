#pragma once

#include "AssetManager.hpp"
#include "assets/StaticMesh.hpp"
#include "assets/AnimatedMesh.hpp"
#include "Components.hpp"
#include <opengl/FrameBuffer.hpp>
#include <opengl/Shader.hpp>

namespace flecs {
    struct world;
}

namespace engine {

class ForwardRenderer {
public:
    enum class Flags : uint32_t {
        enableParallaxMapping = 0x1,
        enableShadowMapping = 0x2
    };

    ForwardRenderer();
    ForwardRenderer(Flags flags);
    ~ForwardRenderer();

    void init(Flags flags);

    void renderWorld(
        float deltaTime, // in miliseconds
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const flecs::world &world,
        const AssetManager &assetManager,
        const FrameBuffer &renderTarget);

private:
    void updateShadowMapLevels(
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const flecs::world &world);
    
    void drawMeshInShadowMaps(
        const engine::components::StaticMesh &meshComponent,
        const glm::mat4 &modelMatrix,
        const AssetManager &assetManager,
        const flecs::world &world) const;
    void drawMeshInShadowMaps(
        const engine::components::AnimatedMesh &meshComponent,
        const engine::components::AnimationPlayer *animationComponent,
        const glm::mat4 &modelMatrix,
        const AssetManager &assetManager,
        const flecs::world &world) const;

    void setMaterialUniforms(
        Shader &shader,
        const Material *material,
        const AssetManager &assetManager,
        int &nextFreeTextureSlot) const;
    void setLightUniforms(
        Shader &shader,
        const flecs::world &world,
        const engine::components::Transform &cameraTransform,
        int &nextFreeTextureSlot) const;

    void drawMesh(
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const engine::components::StaticMesh &meshComponent,
        const engine::components::Material *materialComponent,
        const glm::mat4 &modelMatrix,
        const AssetManager &assetManager,
        const FrameBuffer &renderTarget,
        int nextFreeTextureSlot) const;

    void drawMesh(
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const engine::components::AnimatedMesh &meshComponent,
        const engine::components::Material *materialComponent,
        const engine::components::AnimationPlayer *animationComponent,
        const glm::mat4 &modelMatrix,
        const AssetManager &assetManager,
        const FrameBuffer &renderTarget,
        int nextFreeTextureSlot) const;

    void drawAABB(
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const engine::components::StaticMesh &meshComponent,
        const glm::mat4 &modelMatrix,
        const AssetManager &assetManager,
        const FrameBuffer &renderTarget) const;
    void drawSkeleton(
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const engine::components::AnimatedMesh &meshComponent,
        const engine::components::AnimationPlayer *animationComponent,
        const glm::mat4 &modelMatrix,
        const AssetManager &assetManager,
        const FrameBuffer &renderTarget) const;

    void drawSkybox(
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const Cubemap &cubemap,
        const FrameBuffer &renderTarget) const;

    struct ShadowMapLevel {
        // ProjectionView of the camera for the shadowMap
        glm::mat4 projectionView;

        // Frustum that the shadowMap level has to contain
        glm::mat4 frustumProjectionMatrix;

        float near, far;
        float cutoffDistance;
        float maxDiagonal;
    };

    Flags m_flags;
    std::shared_ptr<Shader> m_staticMeshShader;      // Get the shaders from the ResourceManager
    std::shared_ptr<Shader> m_animatedMeshShader;    // so they arent recreated with multiple
    std::shared_ptr<Shader> m_staticShadowMapShader; // renderers
    std::shared_ptr<Shader> m_animatedShadowMapShader;
    std::shared_ptr<Shader> m_skyboxShader;
    std::shared_ptr<Shader> m_cubeLinesShader;

    std::shared_ptr<VertexArray> m_cubeVertexArray;
    std::shared_ptr<VertexArray> m_cubeVertexArrayForLines;
    std::shared_ptr<VertexArray> m_linesBatchVertexArray;
    mutable std::shared_ptr<VertexBuffer> m_linesBatchVertexBuffer;
    mutable std::vector<glm::vec3> m_linesBatchVertices;
};

inline ForwardRenderer::Flags operator|(ForwardRenderer::Flags a, ForwardRenderer::Flags b)
{
    return ForwardRenderer::Flags(std::to_underlying(a) | std::to_underlying(b));
}

inline ForwardRenderer::Flags operator&(ForwardRenderer::Flags a, ForwardRenderer::Flags b)
{
    return ForwardRenderer::Flags(std::to_underlying(a) & std::to_underlying(b));
}

} // namespace engine
