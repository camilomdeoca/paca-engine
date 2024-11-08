#pragma once

#include "engine/PerspectiveCamera.hpp"
#include "engine/World.hpp"
#include "opengl/FrameBuffer.hpp"
#include "opengl/Shader.hpp"

// TODO: make this a class and then pass a shared ptr to the beginScene() function so its ensured
// that it exists until we store it (until endScene())

class ForwardRenderer {
public:
    struct Parameters {
        enum FlagsMasks : uint32_t {
            enableParallaxMapping = 0x1,
            enableShadowMapping = 0x2
        };
        // size to render to
        uint32_t width = 800, height = 600;
        uint32_t shadowMapSize = 512;

        // for shadowMap levels for example and in the future model LODs
        // all values should be between 0.0f and 1.0f and in order
        std::vector<float> viewFrustumSplits = { 0.0f, 1.0f };

        uint32_t flags = 0;
    };

    ForwardRenderer();
    ForwardRenderer(const Parameters &parameters);
    ~ForwardRenderer();

    void init(const Parameters &parameters);

    void resize(uint32_t width, uint32_t height);

    void renderWorld(const PerspectiveCamera &camera, const World &world);

private:
    void updateShadowMapLevels(const PerspectiveCamera &camera, const World &world);
    
    void drawMeshInShadowMaps(const Mesh &mesh, const glm::mat4 &modelMatrix, const World &world) const;
    void drawMesh(const PerspectiveCamera &camera, const Mesh &mesh, const glm::mat4 &modelMatrix, const World &world) const;
    void drawModelInShadowMaps(const PerspectiveCamera &camera, const Model &model, const World &world) const;
    void drawModel(const PerspectiveCamera &camera, const Model &model, const World &world) const;

    void drawSkybox(const PerspectiveCamera &camera, const Texture &cubemap) const;

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

    std::shared_ptr<Shader> m_staticMeshShader; // Get this from the ResourceManager
                                                // so it isnt recreated with multiple renderers

    Parameters m_parameters;
public:
    std::vector<ShadowMapLevel> m_shadowMapLevels;
private:
    std::shared_ptr<Shader> m_shadowMapShader;
    std::shared_ptr<Shader> m_skyboxShader;
    std::shared_ptr<VertexArray> m_cubeVertexArray;
    uint32_t m_shadowMapSize;
};
