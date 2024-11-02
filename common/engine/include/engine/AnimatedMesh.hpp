#pragma once

#include "engine/Animation.hpp"
#include "engine/Mesh.hpp"

class AnimatedMesh : public Mesh {
public:
    AnimatedMesh(const std::vector<uint8_t> &vertices, 
         const std::vector<uint32_t> &indices, 
         std::shared_ptr<Material> material, 
         std::vector<std::shared_ptr<Animation>> animations, 
         Skeleton &&skeleton);
    virtual ~AnimatedMesh();

    bool isAnimated() const override { return true; }
    const std::shared_ptr<VertexArray> &getVertexArray() const override { return m_vertex_array; }
    const std::shared_ptr<Material> getMaterial() const override { return m_material; }

    const Skeleton &getSkeleton() const { return m_skeleton; }
    const std::vector<std::shared_ptr<Animation>> &getAnimations() const { return m_animations; }
    const Animation &getCurrentAnimation() const { return *m_currentAnimation; }
    const std::vector<glm::mat4> getCurrentAnimationTransformations() const;
    void updateAnimation(float ms) { m_animationProgress += ms / m_currentAnimation->getTicksPerSecond(); }

private:
    std::shared_ptr<VertexArray> m_vertex_array;
    std::shared_ptr<VertexBuffer> m_vertex_buffer;
    std::shared_ptr<IndexBuffer> m_index_buffer;

    std::shared_ptr<Material> m_material;
    float m_animationProgress = 0.0f; // in ticks
    std::vector<std::shared_ptr<Animation>> m_animations;
    std::shared_ptr<Animation> m_currentAnimation;
    Skeleton m_skeleton;
};
