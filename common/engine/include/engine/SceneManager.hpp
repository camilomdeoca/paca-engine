#pragma once

#include <flecs.h>

namespace paca::fileformats {
    struct Scene;
}

namespace engine {

class SceneManager {
public:
    void loadScene(const paca::fileformats::Scene &scene);

    flecs::world &getFlecsWorld() { return m_world; }

private:
    flecs::world m_world;
};

}
