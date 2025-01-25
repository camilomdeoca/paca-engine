#include "engine/assets/Animation.hpp"

#include "utils/Assert.hpp"

#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/matrix.hpp>
#include <limits>

std::vector<glm::mat4> Animation::getTransformations(float time, const Skeleton &skeleton) const
{
    std::vector<glm::mat4> result(m_boneKeyframes.size(), glm::mat4(1.0f));
    std::vector<glm::mat4> boneTransforms(m_boneKeyframes.size());

    for (BoneID boneId = 0; boneId < m_boneKeyframes.size(); boneId++)
    {
        const Bone &bone = skeleton.bones[boneId];
        bool isRoot = bone.parentID == std::numeric_limits<uint32_t>::max();

        ASSERT_MSG(bone.parentID < m_boneKeyframes.size() || isRoot, "Bone parent does not exist (issue in model file)");
        ASSERT_MSG(boneId > bone.parentID || isRoot, "Bone appears before parent (issue in model file)");

        glm::mat4 translation = glm::translate(glm::mat4(1.0f), getPosition(boneId, time));
        glm::mat4 rotation = glm::mat4_cast(getRotation(boneId, time));
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), getScale(boneId, time));
        glm::mat4 animation = translation * rotation * scaling;

        boneTransforms[boneId] = isRoot
            ? animation
            : boneTransforms[bone.parentID] * animation;

        result[boneId] = boneTransforms[boneId] * bone.offsetMatrix;
    }

    return result;
}

glm::vec3 Animation::getPosition(BoneID bone, float time) const
{
    for (size_t i = 1; i < m_boneKeyframes[bone].positions.size(); i++)
    {
        const PositionKeyFrame &next = m_boneKeyframes[bone].positions[i];
        const PositionKeyFrame &prev = m_boneKeyframes[bone].positions[i-1];
        if (next.time >= time)
        {
            float delta = next.time - prev.time;
            float ratio = (time - prev.time) / delta;
            return glm::mix(prev.position, next.position, ratio);
        }
    }
    if (m_boneKeyframes[bone].positions.empty()) return {0.0f, 0.0f, 0.0f};
    ASSERT_MSG(time < getDuration(), "There is no position keyframe for time {}/{}", time, getDuration());
    return m_boneKeyframes[bone].positions.back().position;
}

glm::quat Animation::getRotation(BoneID bone, float time) const
{
    for (size_t i = 1; i < m_boneKeyframes[bone].rotations.size(); i++)
    {
        const RotationKeyFrame &next = m_boneKeyframes[bone].rotations[i];
        const RotationKeyFrame &prev = m_boneKeyframes[bone].rotations[i-1];
        if (next.time >= time)
        {
            float delta = next.time - prev.time;
            float ratio = (time - prev.time) / delta;
            return glm::slerp(prev.quaternion, next.quaternion, ratio);
        }
    }
    if (m_boneKeyframes[bone].rotations.empty()) return glm::identity<glm::quat>();
    ASSERT_MSG(time < getDuration(), "There is no rotation keyframe for time {}/{}", time, getDuration());
    return m_boneKeyframes[bone].rotations.back().quaternion;
}

glm::vec3 Animation::getScale(BoneID bone, float time) const
{
    for (size_t i = 1; i < m_boneKeyframes[bone].positions.size(); i++)
    {
        const ScaleKeyFrame &next = m_boneKeyframes[bone].scalings[i];
        const ScaleKeyFrame &prev = m_boneKeyframes[bone].scalings[i-1];
        if (next.time >= time)
        {
            float delta = next.time - prev.time;
            float ratio = (time - prev.time) / delta;
            return glm::mix(prev.scale, next.scale, ratio);
        }
    }
    if (m_boneKeyframes[bone].scalings.empty()) return {1.0f, 1.0f, 1.0f};
    ASSERT_MSG(time < getDuration(), "There is no scale keyframe for time {}/{}", time, getDuration());
    return m_boneKeyframes[bone].scalings.back().scale;
}
