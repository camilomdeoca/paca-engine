#pragma once

#include "Animation.hpp"
#include "Mesh.hpp"
#include "engine/IdTypes.hpp"

class AnimatedMesh : public Mesh {
public:
    AnimatedMesh(const std::vector<uint8_t> &vertices, 
         const std::vector<uint32_t> &indices, 
         std::vector<AnimationId> animationIds, 
         Skeleton &&skeleton);
    virtual ~AnimatedMesh();

    const VertexArray &getVertexArray() const override { return *m_vertex_array; }

    const Skeleton &getSkeleton() const { return m_skeleton; }
    const std::vector<AnimationId> &getAnimationIds() const { return m_animationIds; }
    const AnimationId getCurrentAnimationId() const { return m_currentAnimationId; }
    const std::vector<glm::mat4> getCurrentAnimationTransformations() const;
    void updateAnimation(float ms) { /* m_animationProgress += ms / m_currentAnimation->getTicksPerSecond(); */ }

private:
    std::shared_ptr<VertexArray> m_vertex_array;
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    float m_animationProgress = 0.0f; // in ticks
    std::vector<AnimationId> m_animationIds;
    AnimationId m_currentAnimationId;
    Skeleton m_skeleton;
};
