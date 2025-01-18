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
        const StaticMesh &mesh,
        const glm::mat4 &modelMatrix,
        const flecs::world &world) const;
    void drawMeshInShadowMaps(
        const AnimatedMesh &mesh,
        const glm::mat4 &modelMatrix,
        const flecs::world &world) const;

    void drawMesh(
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const StaticMesh &mesh,
        const Material *material,
        const glm::mat4 &modelMatrix,
        const flecs::world &world,
        const AssetManager &assetManager,
        const FrameBuffer &renderTarget) const;

    void drawAABB(
        const engine::components::Transform &cameraTransform,
        const engine::components::Camera &camera,
        const AxisAlignedBoundingBox &aabb,
        const glm::mat4 &modelMatrix,
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

    std::shared_ptr<Shader> m_staticMeshShader; // Get this from the ResourceManager
                                                // so it isnt recreated with multiple renderers

    Flags m_flags;
    std::shared_ptr<Shader> m_shadowMapShader;
    std::shared_ptr<Shader> m_skyboxShader;
    std::shared_ptr<Shader> m_cubeLinesShader;
    std::shared_ptr<VertexArray> m_cubeVertexArray;
    std::shared_ptr<VertexArray> m_cubeVertexArrayForLines;
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
