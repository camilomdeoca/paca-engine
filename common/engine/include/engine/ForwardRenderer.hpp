#pragma once

#include <engine/NewResourceManager.hpp>
#include <engine/PerspectiveCamera.hpp>
#include <engine/StaticMesh.hpp>
#include <engine/AnimatedMesh.hpp>
#include <opengl/FrameBuffer.hpp>
#include <opengl/Shader.hpp>

namespace flecs {
    struct world;
}

namespace engine {

class ForwardRenderer {
public:
    struct Parameters {
        enum FlagsMasks : uint32_t {
            enableParallaxMapping = 0x1,
            enableShadowMapping = 0x2
        };
        // size to render to
        uint32_t width = 800, height = 600;
        uint32_t flags = 0;
    };

    ForwardRenderer();
    ForwardRenderer(const Parameters &parameters);
    ~ForwardRenderer();

    void init(const Parameters &parameters);

    void resize(uint32_t width, uint32_t height);
    void setRenderTarget(FrameBuffer &renderTarget) { m_renderTarget = &renderTarget; }

    void renderWorld(const PerspectiveCamera &camera, const flecs::world &world, const NewResourceManager &resourceManager);

private:
    void updateShadowMapLevels(const PerspectiveCamera &camera, const flecs::world &world);
    
    void drawMeshInShadowMaps(
        const StaticMesh &mesh,
        const glm::mat4 &modelMatrix,
        const flecs::world &world) const;
    void drawMeshInShadowMaps(
        const AnimatedMesh &mesh,
        const glm::mat4 &modelMatrix,
        const flecs::world &world) const;

    void drawMesh(
        const PerspectiveCamera &camera,
        const StaticMesh &mesh,
        const Material *material,
        const glm::mat4 &modelMatrix,
        const flecs::world &world,
        const NewResourceManager &resourceManager) const;
    void drawMesh(
        const PerspectiveCamera &camera,
        const AnimatedMesh *mesh,
        const Material &material,
        const glm::mat4 &modelMatrix,
        const flecs::world &world,
        const NewResourceManager &resourceManager) const;

    void drawSkybox(const PerspectiveCamera &camera, const Texture &cubemap) const;

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

    Parameters m_parameters;
    std::shared_ptr<Shader> m_shadowMapShader;
    std::shared_ptr<Shader> m_skyboxShader;
    std::shared_ptr<VertexArray> m_cubeVertexArray;
    FrameBuffer *m_renderTarget;
};

} // namespace engine
