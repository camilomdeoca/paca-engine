#pragma once

#include "engine/IdTypes.hpp"

namespace engine::components {

struct Material
{
    NAME("Material")
    FIELDS(id)
    FIELD_NAMES("id")
    MaterialId id;
};

}
