#pragma once

#include "engine/IdTypes.hpp"

namespace engine::components {

struct AnimatedMesh
{
    NAME("AnimatedMesh")
    FIELDS(id)
    FIELD_NAMES("id")
    AnimatedMeshId id;
};

}
