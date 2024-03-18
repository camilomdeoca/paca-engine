#include "Mesh.hpp"

#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/VertexBuffer.hpp"
#include "util/MeshLoader.hpp"

#include <memory>
#include <optional>
#include <vector>

Mesh::Mesh(const std::string &path)
{
    std::optional<MeshData> data = MeshLoader::loadObj(path);

    if (!data.has_value()) {
        fprintf(stderr, "Error reading mesh obj file: %s\n", path.c_str());
        exit(1);
    }

    m_vertex_array = std::make_shared<VertexArray>();

    m_vertex_buffer = std::make_shared<VertexBuffer>(data.value().vertices.data(), data.value().vertices.size() * sizeof(float));
    m_vertex_buffer->setLayout({
        {ShaderDataType::float3, "a_position"},
        {ShaderDataType::float3, "a_normal"},
        {ShaderDataType::float2, "a_uvCoords"}
    });

    m_vertex_array->addVertexBuffer(m_vertex_buffer);

    m_index_buffer = std::make_shared<IndexBuffer>(data.value().indices.data(), data.value().indices.size());

    m_vertex_array->setIndexBuffer(m_index_buffer);
}

Mesh::Mesh(const std::vector<float> &vertices, const std::vector<uint32_t> &indices, std::shared_ptr<Material> material)
    : m_material(material)
{
    m_vertex_array = std::make_shared<VertexArray>();

    m_vertex_buffer = std::make_shared<VertexBuffer>(vertices.data(), vertices.size() * sizeof(float));
    m_vertex_buffer->setLayout({
        {ShaderDataType::float3, "a_position"},
        {ShaderDataType::float3, "a_normal"},
        {ShaderDataType::float2, "a_uvCoords"}
    });

    m_vertex_array->addVertexBuffer(m_vertex_buffer);

    m_index_buffer = std::make_shared<IndexBuffer>(indices.data(), indices.size());

    m_vertex_array->setIndexBuffer(m_index_buffer);
}

Mesh::~Mesh()
{}
