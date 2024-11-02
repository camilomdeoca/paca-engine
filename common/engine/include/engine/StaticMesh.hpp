#pragma once

#include "engine/Mesh.hpp"

class StaticMesh : public Mesh {
public:
    StaticMesh(const std::vector<uint8_t> &vertices, 
         const std::vector<uint32_t> &indices, 
         std::shared_ptr<Material> material);
    virtual ~StaticMesh();

    bool isAnimated() const override { return false; }
    const std::shared_ptr<VertexArray> &getVertexArray() const override { return m_vertex_array; }
    const std::shared_ptr<Material> getMaterial() const override { return m_material; }

private:
    std::shared_ptr<VertexArray> m_vertex_array;
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    std::shared_ptr<Material> m_material;
};
