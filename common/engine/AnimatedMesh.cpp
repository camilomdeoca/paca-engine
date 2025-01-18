#include "engine/assets/AnimatedMesh.hpp"

AnimatedMesh::AnimatedMesh(
    const std::vector<uint8_t> &vertices,
    const std::vector<uint32_t> &indices,
    std::vector<AnimationId> animationIds,
    Skeleton &&skeleton)
    : m_animationIds(animationIds),
      m_skeleton(std::move(skeleton))
{
    m_vertex_array = std::make_shared<VertexArray>();

    m_vertex_buffer = std::make_shared<VertexBuffer>(vertices.data(), vertices.size());
    m_vertex_buffer->setLayout({
        {ShaderDataType::float3, "a_position"},
        {ShaderDataType::float3, "a_normal"},
        {ShaderDataType::float3, "a_tangent"},
        {ShaderDataType::float2, "a_uvCoords"},
        {ShaderDataType::uint4,  "a_boneIds"},
        {ShaderDataType::float4, "a_boneWeights"}
    });

    m_vertex_array->addVertexBuffer(m_vertex_buffer);

    m_index_buffer = std::make_shared<IndexBuffer>(indices.data(), indices.size());

    m_vertex_array->setIndexBuffer(m_index_buffer);
    m_currentAnimationId = m_animationIds[0];
}

AnimatedMesh::~AnimatedMesh()
{}

const std::vector<glm::mat4> AnimatedMesh::getCurrentAnimationTransformations() const
{
    return {};
    /* return m_currentAnimation->getTransformations(m_animationProgress, m_skeleton); */
}
