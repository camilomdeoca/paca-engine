#pragma once

#include "Animation.hpp"
#include "Mesh.hpp"

class AnimatedMesh : public Mesh {
public:
    // We are assuming that this struct has no padding in between the members
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 texture;
        glm::vec<4, uint32_t> boneIDs;
        glm::vec4 boneWeights;
    };

    AnimatedMesh(std::span<const Vertex> vertices, 
         std::span<const uint32_t> indices, 
         Skeleton &&skeleton);
    virtual ~AnimatedMesh();

    const VertexArray &getVertexArray() const override { return *m_vertex_array; }

    const Skeleton &getSkeleton() const { return m_skeleton; }

private:
    std::shared_ptr<VertexArray> m_vertex_array;
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    Skeleton m_skeleton;
};
