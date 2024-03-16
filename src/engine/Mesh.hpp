#pragma once

#include "opengl/IndexBuffer.hpp"
#include "opengl/Texture.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/VertexBuffer.hpp"

#include <memory>

class Mesh {
public:
    Mesh(const std::string &path);
    ~Mesh();

    std::shared_ptr<VertexArray> getVertexArray() { return m_vertex_array; }

private:
    std::shared_ptr<VertexArray> m_vertex_array;
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    std::shared_ptr<Texture> m_texture;
};
