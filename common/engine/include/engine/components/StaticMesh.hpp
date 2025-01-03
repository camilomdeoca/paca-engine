#pragma once

#include "engine/IdTypes.hpp"

namespace engine::components {

struct StaticMesh
{
    NAME("StaticMesh")
    FIELDS(id)
    FIELD_NAMES("id")
    StaticMeshId id;
};

}
