#pragma once

#include "Mesh.hpp"

#include "engine/AxisAlignedBoundingBox.hpp"

class StaticMesh : public Mesh
{
public:
    // We are assuming that this struct has no padding in between the members
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 texture;
    };

    StaticMesh(
        std::span<const Vertex> vertices,
        std::span<const uint32_t> indices,
        const AxisAlignedBoundingBox &aabb);
    virtual ~StaticMesh();

    const VertexArray &getVertexArray() const override { return *m_vertex_array; }
    const AxisAlignedBoundingBox &getAABB() const { return m_aabb; }

private:
    std::shared_ptr<VertexArray> m_vertex_array;
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    AxisAlignedBoundingBox m_aabb;
};
