#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Serializer.hpp"

#include <string>
#include <vector>
#include <array>

namespace paca::fileformats {

enum class VertexType : uint32_t {
    float3pos_float3norm_float2texture = 1,
    float3pos_float3norm_float3tang_float2texture = 2,
    float3pos_float3norm_float3tang_float2texture_int4boneIds_float4boneWeights = 3
};

enum class IndexType : uint32_t {
    no_indices = 0,
    uint32bit = 1,
};

struct Bone {
    uint32_t parentID;
    glm::mat4 offsetMatrix;
};

struct Skeleton {
    std::vector<Bone> bones;
    std::vector<std::string> boneNames;
};

struct Mesh {
    VertexType vertexType;
    IndexType indexType;
    std::vector<char> vertices;
    std::vector<uint32_t> indices;
    std::string materialName;
    std::vector<std::string> animations;
    Skeleton skeleton;
};

struct Model {
    std::vector<Mesh> meshes;
    std::string name;
};

struct PositionKeyFrame {
    float time;
    glm::vec3 position;
};

struct RotationKeyFrame {
    float time;
    glm::quat quaternion;
};

struct ScaleKeyFrame {
    float time;
    glm::vec3 scale;
};

struct BoneKeyFrames {
    std::vector<PositionKeyFrame> positions;
    std::vector<RotationKeyFrame> rotations;
    std::vector<ScaleKeyFrame> scalings;
};

struct Animation {
    // Duration of the animation in ticks
    float duration;
    uint32_t ticksPerSecond;
    std::string name;

    // Keyframes of each bone
    std::vector<BoneKeyFrames> keyframes;
};

namespace TextureType {
    enum Type : uint32_t {
        none = 0,
        diffuse = 1,
        specular = 2,
        normal = 3,
        depth = 4,
        last = 5
    };
}

struct Texture {
    uint32_t width, height;
    uint32_t channels;
    bool isCubeMap;
    std::vector<char> pixelData;
};

struct Material {
    std::array<std::vector<Texture>, TextureType::last> textures;
    std::string name;
};

struct ResourcePack {
    std::vector<Model> models;
    std::vector<Material> materials;
    std::vector<Animation> animations;
};

size_t vertexTypeToSize(VertexType type);
size_t indexTypeToSize(IndexType type);

} // namespace paca::fileformats
