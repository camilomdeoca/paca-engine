#pragma once

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

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
    std::vector<uint8_t> vertices;
    std::vector<uint32_t> indices;
    std::string materialName;
    std::vector<std::string> animations;
    Skeleton skeleton;
};

struct Model {
    std::string name;
    std::vector<Mesh> meshes;
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
    std::string name;
    float duration;
    uint32_t ticksPerSecond;

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
    std::string name;
    uint32_t width, height;
    uint32_t channels;
    bool isCubeMap;
    std::vector<uint8_t> pixelData;
};

struct Material {
    std::string name;
    std::array<std::vector<std::string>, TextureType::last> textures;
};

struct AssetPack {
    std::vector<Model> models;
    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<Animation> animations;
};

struct Object {
    std::string model;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct Light {
    glm::vec3 color;
    float intensity;
};

struct DirectionalLight {
    Light light;
    glm::vec3 direction;
};

struct PointLight {
    Light light;
    glm::vec3 position;
    float attenuation;
    std::string model;
};

struct Scene {
    std::vector<Object> objects;
    DirectionalLight directionalLight;
    std::vector<PointLight> pointLights;
};

size_t vertexTypeToSize(VertexType type);
size_t indexTypeToSize(IndexType type);

paca::fileformats::AssetPack combine(std::vector<std::reference_wrapper<paca::fileformats::AssetPack>> resourcePacks);

} // namespace paca::fileformats
