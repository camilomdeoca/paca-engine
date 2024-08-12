#pragma once

#include <pacaread/pacaread.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>

using BoneKeyFrames = paca_format::BoneKeyFrames;

using PositionKeyFrame = paca_format::PositionKeyFrame;
using RotationKeyFrame = paca_format::RotationKeyFrame;
using ScaleKeyFrame = paca_format::ScaleKeyFrame;

using Skeleton = paca_format::Skeleton;
using Bone = paca_format::Bone;

using BoneID = uint32_t; // Maybe move this to the Mesh class

class Animation {
public:
    Animation(float duration,
              uint32_t ticksPerSecond,
              const std::string &name,
              std::vector<BoneKeyFrames> &&boneKeyframes)
        : m_duration(duration),
          m_ticksPerSecond(ticksPerSecond),
          m_name(name),
          m_boneKeyframes(std::move(boneKeyframes)) {}

    // the time parameter need to be in ticks
    std::vector<glm::mat4> getTransformations(float time, const Skeleton &skeleton) const;
    float getDuration() const { return m_duration; }
    float getTicksPerSecond() const { return m_ticksPerSecond; }

private:
    glm::vec3 getPosition(BoneID bone, float time) const;
    glm::quat getRotation(BoneID bone, float time) const;
    glm::vec3 getScale(BoneID bone, float time) const;

    float m_duration; // in ticks
    uint32_t m_ticksPerSecond;
    std::string m_name;

    std::vector<BoneKeyFrames> m_boneKeyframes;
};
