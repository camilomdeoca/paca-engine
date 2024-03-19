#pragma once

#include "engine/Camera.hpp"
#include "engine/Light.hpp"
#include "engine/Mesh.hpp"
#include "engine/Model.hpp"

class Renderer {
public:
    static void init();

    static void beginScene(const Camera &camera, const std::vector<std::shared_ptr<Light>> &lights);
    static void endScene();

    static void drawMesh(Mesh &mesh, const glm::mat4 &modelMatrix);
    static void drawModel(Model &model);
};

