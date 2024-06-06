#pragma once

#include "engine/Material.hpp"
#include "opengl/VertexArray.hpp"

#include <memory>

class Mesh {
public:
    virtual bool isAnimated() const = 0;

    virtual const std::shared_ptr<VertexArray> &getVertexArray() const = 0;
    virtual const std::shared_ptr<Material> getMaterial() const = 0;
};
