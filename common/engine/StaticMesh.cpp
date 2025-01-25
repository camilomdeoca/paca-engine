#include "engine/assets/StaticMesh.hpp"

StaticMesh::StaticMesh(
    std::span<const Vertex> vertices,
    std::span<const uint32_t> indices,
    const AxisAlignedBoundingBox &aabb)
    : m_aabb(aabb)
{
    m_vertex_array = std::make_shared<VertexArray>();

    m_vertex_buffer = std::make_shared<VertexBuffer>(
        vertices.data(),
        vertices.size() * sizeof(vertices[0]));
    m_vertex_buffer->setLayout({
        {ShaderDataType::float3, "a_position"},
        {ShaderDataType::float3, "a_normal"},
        {ShaderDataType::float3, "a_tangent"},
        {ShaderDataType::float2, "a_uvCoords"}
    });

    m_vertex_array->addVertexBuffer(m_vertex_buffer);

    m_index_buffer = std::make_shared<IndexBuffer>(indices.data(), indices.size());

    m_vertex_array->setIndexBuffer(m_index_buffer);
}

StaticMesh::~StaticMesh()
{}
