#pragma once

#include "engine/Camera.hpp"
#include "engine/Mesh.hpp"
#include "engine/Model.hpp"

class Renderer {
public:
    static void init();

    static void beginScene(const Camera &camera);
    static void endScene();

    static void drawMesh(Mesh &mesh);
    static void drawModel(Model &model);
};

