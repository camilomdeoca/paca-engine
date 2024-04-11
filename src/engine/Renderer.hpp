#pragma once

#include "engine/Light.hpp"
#include "engine/Mesh.hpp"
#include "engine/Model.hpp"
#include "engine/PerspectiveCamera.hpp"
#include "opengl/Texture.hpp"

#include <memory>

// TODO: make this a class and then pass a shared ptr to the beginScene() function so its ensured
// that it exists until we store it (until endScene())
struct RenderEnvironment {
    std::vector<std::shared_ptr<PointLight>> *pointLights;
    std::shared_ptr<DirectionalLight> directionalLight; // Only 1 for now
    std::shared_ptr<Texture> skybox;
};

struct RendererParameters {
    // size to render to
    uint32_t width, height;
    uint32_t shadowMapSize;

    // for shadowMap levels for example and in the future model LODs
    // all values should be between 0.0f and 1.0f and in order
    std::vector<float> viewFrustumSplits;
};

class Renderer {
public:
    static void init(RendererParameters parameters);

    static void resize(uint32_t width, uint32_t height);

    static void beginScene(const PerspectiveCamera &camera, const RenderEnvironment &environment);
    static void endScene();

    static void drawMesh(Mesh &mesh, const glm::mat4 &modelMatrix);
    static void drawModel(Model &model);
private:
    static void createFramebuffers();
};


// DEBUG
#ifdef DEBUG
std::vector<std::shared_ptr<Texture>> getShadowMaps();
#endif // DEBUG
