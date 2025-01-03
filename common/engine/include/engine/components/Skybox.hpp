#pragma once

#include "engine/IdTypes.hpp"
namespace engine::components {

struct Skybox
{
    NAME("Skybox")
    FIELDS(id)
    FIELD_NAMES("id")
    CubeMapId id;
};

}
