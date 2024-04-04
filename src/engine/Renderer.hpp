#pragma once

#include "engine/Camera.hpp"
#include "engine/Light.hpp"
#include "engine/Mesh.hpp"
#include "engine/Model.hpp"

#include <memory>

struct RenderEnvironment {
    std::vector<std::shared_ptr<Light>> *pointLights;
};

struct RendererParameters {
    // size to render to
    uint32_t width, height;
};

class Renderer {
public:
    static void init(RendererParameters parameters);

    static void resize(uint32_t width, uint32_t height);

    static void beginScene(const Camera &camera, const RenderEnvironment &environment);
    static void endScene();

    static void drawMesh(Mesh &mesh, const glm::mat4 &modelMatrix);
    static void drawModel(Model &model);
private:
    static void createFramebuffers();
};

