#pragma once

#include <flecs.h>
#include <cstdint>
#include <functional>

class PerspectiveCamera;
class NewResourceManager;
class FrameBuffer;

namespace ui {

struct EditorContext
{
    PerspectiveCamera &camera;
    NewResourceManager &resourceManager;
    FrameBuffer &renderTarget;
    std::function<void(uint32_t, uint32_t)> resizeCallback;
    flecs::world &world;
    flecs::entity selectedEntity;
};

void draw(EditorContext &context);

} // namespace ui
