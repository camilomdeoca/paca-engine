#include "Mesh.hpp"

#include "opengl/IndexBuffer.hpp"
#include "opengl/VertexArray.hpp"
#include "opengl/VertexBuffer.hpp"
#include "util/MeshLoader.hpp"

#include <memory>
#include <optional>

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
        {ShaderDataType::float2, "a_uvCoords"},
        {ShaderDataType::float3, "a_normal"}
    });

    m_vertex_array->addVertexBuffer(m_vertex_buffer);

    m_index_buffer = std::make_shared<IndexBuffer>(data.value().indices.data(), data.value().indices.size());

    m_vertex_array->setIndexBuffer(m_index_buffer);
}

Mesh::~Mesh()
{}
