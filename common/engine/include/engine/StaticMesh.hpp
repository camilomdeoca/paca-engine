#pragma once

#include "Mesh.hpp"

class StaticMesh : public Mesh {
public:
    StaticMesh(const std::vector<uint8_t> &vertices, const std::vector<uint32_t> &indices);
    virtual ~StaticMesh();

    const VertexArray &getVertexArray() const override { return *m_vertex_array; }

private:
    std::shared_ptr<VertexArray> m_vertex_array;
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;
};
