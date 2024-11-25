#pragma once

#include "opengl/VertexArray.hpp"

class Mesh {
public:
    virtual const VertexArray &getVertexArray() const = 0;
};
