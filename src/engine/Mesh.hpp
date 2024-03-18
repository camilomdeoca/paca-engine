#pragma once

#include "engine/Material.hpp"
#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/VertexBuffer.hpp"

#include <memory>

class Mesh {
public:
    Mesh(const std::string &path);
    Mesh(const std::vector<float> &vertices, const std::vector<uint32_t> &indices, std::shared_ptr<Material> material);
    ~Mesh();

    const std::shared_ptr<VertexArray> &getVertexArray() { return m_vertex_array; }
    const std::shared_ptr<Material> getMaterial() { return m_material; }

private:
    std::shared_ptr<VertexArray> m_vertex_array;
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    std::shared_ptr<Material> m_material;
};
