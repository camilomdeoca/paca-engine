#pragma once

#include <serializers/Reflection.hpp>

#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <variant>
#include <vector>
#include <array>

namespace paca::fileformats {

using EntityId = uint32_t;

using StaticMeshId = uint32_t;
using AnimatedMeshId = uint32_t;
using TextureId = uint32_t;
using CubeMapId = uint32_t;
using MaterialId = uint32_t;
using AnimationId = uint32_t;
using FontId = uint32_t;

struct Bone {
    NAME("Bone")
    FIELDS(parentID, offsetMatrix)
    FIELD_NAMES("parentID", "offsetMatrix")
    uint32_t parentID;
    glm::mat4 offsetMatrix;
};

struct Skeleton {
    NAME("Skeleton")
    FIELDS(bones, boneNames)
    FIELD_NAMES("bones", "boneNames")
    std::vector<Bone> bones;
    std::vector<std::string> boneNames;
};

struct StaticMesh {
    NAME("StaticMesh")
    FIELDS(name, id, vertices, indices, materialId)
    FIELD_NAMES("name", "id", "vertices", "indices", "materialId")
    std::string name;
    StaticMeshId id;
    std::vector<uint8_t> vertices;
    std::vector<uint32_t> indices;
    MaterialId materialId;

    // @ glm::vec3 position
    // @ glm::vec3 normal
    // @ glm::vec3 tangent
    // @ glm::vec2 texture
    static constexpr size_t vertex_size = (3+3+3+2)*sizeof(float);
};

struct AnimatedMesh {
    NAME("AnimatedMesh")
    FIELDS(name, id, vertices, indices, materialId, animations, skeleton)
    FIELD_NAMES("name", "id", "vertices", "indices", "materialId", "animations", "skeleton")
    std::string name;
    AnimatedMeshId id;
    std::vector<uint8_t> vertices;
    std::vector<uint32_t> indices;
    MaterialId materialId;
    std::vector<AnimationId> animations;
    Skeleton skeleton;

    // @ glm::vec3 position
    // @ glm::vec3 normal
    // @ glm::vec3 tangent
    // @ glm::vec2 texture
    // @ glm::vec<4, uint32_t> boneIDs
    // @ glm::vec4 boneWeights
    static constexpr size_t vertex_size = (3+3+3+2)*sizeof(float) + 4*sizeof(int32_t) + 4*sizeof(float);
};

struct PositionKeyFrame {
    NAME("PositionKeyFrame")
    FIELDS(time, position)
    FIELD_NAMES("time", "position")
    float time;
    glm::vec3 position;
};

struct RotationKeyFrame {
    NAME("RotationKeyFrame")
    FIELDS(time, quaternion)
    FIELD_NAMES("time", "quaternion")
    float time;
    glm::quat quaternion;
};

struct ScaleKeyFrame {
    NAME("ScaleKeyFrame")
    FIELDS(time, scale)
    FIELD_NAMES("time", "scale")
    float time;
    glm::vec3 scale;
};

struct BoneKeyFrames {
    NAME("BoneKeyFrames")
    FIELDS(positions, rotations, scalings)
    FIELD_NAMES("positions", "rotations", "scalings")
    std::vector<PositionKeyFrame> positions;
    std::vector<RotationKeyFrame> rotations;
    std::vector<ScaleKeyFrame> scalings;
};

struct Animation {
    NAME("Animation")
    FIELDS(name, id, duration, ticksPerSecond, keyframes)
    FIELD_NAMES("name", "id", "duration", "ticksPerSecond", "keyframes")
    // Duration of the animation in ticks
    std::string name;
    AnimationId id;
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
    NAME("Texture")
    FIELDS(name, id, width, height, channels, pixelData)
    FIELD_NAMES("name", "id", "width", "height", "channels", "pixelData")
    std::string name;
    TextureId id;
    uint32_t width, height;
    uint32_t channels;
    std::vector<uint8_t> pixelData;
};

struct CubeMap {
    NAME("CubeMap")
    FIELDS(name, id, width, height, channels, pixelData)
    FIELD_NAMES("name", "id", "width", "height", "channels", "pixelData")
    std::string name;
    TextureId id;
    uint32_t width, height;
    uint32_t channels;
    std::vector<uint8_t> pixelData;
};

struct Material {
    NAME("Material")
    FIELDS(name, id, textures)
    FIELD_NAMES("name", "id", "textures")
    std::string name;
    MaterialId id;
    std::array<std::vector<TextureId>, TextureType::last> textures;
};

struct GlyphData
{
    FIELDS(characterCode, textureCoords, size, advance, offset)
    FIELD_NAMES("characterCode", "textureCoords", "size", "advance", "offset")
    uint32_t characterCode;
    glm::vec<2, int32_t> textureCoords;
    glm::vec<2, uint16_t> size;
    glm::vec<2, int16_t> advance;
    glm::vec<2, int16_t> offset;
};

struct Font {
    NAME("Font")
    FIELDS(name, id, fontHeight, glyphs, atlasTextureId)
    FIELD_NAMES("name", "id", "fontHeight", "glyphs", "atlasTextureId")
    std::string name;
    FontId id;
    uint16_t fontHeight;
    std::vector<GlyphData> glyphs;
    TextureId atlasTextureId;
};

struct AssetPack {
    NAME("AssetPack")
    FIELDS(staticMeshes, animatedMeshes, materials, textures, cubeMaps, animations, fonts)
    FIELD_NAMES("staticMeshes", "animatedMeshes", "materials", "textures", "cubeMaps", "animations", "fonts")
    std::vector<StaticMesh> staticMeshes;
    std::vector<AnimatedMesh> animatedMeshes;
    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<CubeMap> cubeMaps;
    std::vector<Animation> animations;
    std::vector<Font> fonts;
};

namespace components {
    struct Transform {
        NAME("Transform")
        FIELDS(position, rotation, scale)
        FIELD_NAMES("position", "rotation", "scale")
        glm::vec3 position = {0.0f, 0.0f, 0.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 scale    = {1.0f, 1.0f, 1.0f};
    };

    struct StaticMesh {
        NAME("StaticMesh")
        FIELDS(id)
        FIELD_NAMES("id")
        StaticMeshId id;
    };

    struct AnimatedMesh {
        NAME("AnimatedMesh")
        FIELDS(id)
        FIELD_NAMES("id")
        AnimatedMeshId id;
    };

    struct DirectionalLight {
        NAME("DirectionalLight")
        FIELDS(color, intensity)
        FIELD_NAMES("color", "intensity")
        glm::vec3 color;
        float intensity;
    };

    struct PointLight {
        NAME("PointLight")
        FIELDS(color, intensity, attenuation)
        FIELD_NAMES("color", "intensity", "attenuation")
        glm::vec3 color;
        float intensity;
        float attenuation;
    };

} // namespace paca::fileformats::components

using Component = std::variant<
    components::Transform,
    components::StaticMesh,
    components::AnimatedMesh,
    components::DirectionalLight,
    components::PointLight
>;

struct Entity {
    NAME("Entity")
    FIELDS(id, components)
    FIELD_NAMES("id", "components")
    EntityId id;
    std::vector<Component> components;
};


struct Scene {
    NAME("Scene")
    FIELDS(entities)
    FIELD_NAMES("entities")
    std::vector<Entity> entities;
};

} // namespace paca::fileformats
