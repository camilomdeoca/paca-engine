#pragma once

#include <engine/IdTypes.hpp>

#include <opengl/FrameBuffer.hpp>

#include <reflection/Reflection.hpp>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace engine::components {

struct Transform
{
    NAME("Transform")
    FIELDS(position, rotation, scale)
    FIELD_NAMES("position", "rotation", "scale")
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale    = {1.0f, 1.0f, 1.0f};

    glm::mat4 getTransform() const
    {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, position);
        transform = glm::scale(transform, scale);
        glm::quat rot(glm::radians(rotation));
        transform = transform * glm::mat4_cast(rot);
        return transform;
    }

    glm::vec3 getDirection() const
    {
        return glm::normalize(glm::vec3(
            cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x)),
            sin(glm::radians(rotation.x)),
            sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x))
        ));
    }

    glm::mat4 getView() const
    {
        return glm::lookAt(position, position + getDirection(), {0.0f, 1.0f, 0.0f});
    }

    glm::mat3 getRotationMat3() const
    {
        return glm::mat3_cast(glm::quat(glm::radians(rotation)));
    }
};

struct Material
{
    NAME("Material")
    FIELDS(id)
    FIELD_NAMES("id")
    MaterialId id = MaterialId::null;
};

struct StaticMesh
{
    NAME("StaticMesh")
    FIELDS(id)
    FIELD_NAMES("id")
    StaticMeshId id = StaticMeshId::null;
};

struct AnimatedMesh
{
    NAME("AnimatedMesh")
    FIELDS(id)
    FIELD_NAMES("id")
    AnimatedMeshId id = AnimatedMeshId::null;
};

struct PointLight
{
    NAME("PointLight")
    FIELDS(color, intensity, attenuation)
    FIELD_NAMES("color", "intensity", "attenuation")
    glm::vec3 color;
    float intensity;
    float attenuation;
};

struct DirectionalLight
{
    NAME("DirectionalLight")
    FIELDS(color, intensity)
    FIELD_NAMES("color", "intensity")
    glm::vec3 color;
    float intensity;
};

constexpr size_t MAX_DIRECTIONAL_LIGHT_SHADOW_MAP_LEVELS = 5;

struct DirectionalLightShadowMap
{
    DirectionalLightShadowMap(
        uint32_t shadowMapSize,
        const std::vector<float> &viewFrustumSplits);

    struct ShadowMapLevel {
        // ProjectionView of the camera for the shadowMap
        glm::mat4 projectionView;

        // Frustum that the shadowMap level has to contain
        glm::mat4 frustumProjectionMatrix;

        float near, far;
        float cutoffDistance;
        float maxDiagonal;
    };

    DirectionalLightShadowMap(const DirectionalLightShadowMap&) = delete;
    DirectionalLightShadowMap& operator=(const DirectionalLightShadowMap&) = delete;

    DirectionalLightShadowMap(DirectionalLightShadowMap &&directionalLightShadowMap);
    DirectionalLightShadowMap& operator=(DirectionalLightShadowMap &&directionalLightShadowMap);
    
    uint32_t shadowMapSize;
    ShadowMapLevel levels[MAX_DIRECTIONAL_LIGHT_SHADOW_MAP_LEVELS];
    uint8_t levelCount;
    FrameBuffer shadowMapAtlasFramebuffer;
};

struct Skybox
{
    NAME("Skybox")
    FIELDS(id)
    FIELD_NAMES("id")
    CubeMapId id = CubeMapId::null;
};

struct Camera
{
    NAME("Camera")
    FIELDS(aspect, fov, near, far)
    FIELD_NAMES("aspect", "fov", "near", "far")
    float aspect = 1.0f;
    float fov = 90.0f;
    float near = 0.1f;
    float far = 100.0f;

    glm::mat4 getProjection() const
    {
        return glm::perspective(glm::radians(fov), aspect, near, far);
    }
};

}

namespace engine::tags {

struct SceneEntityTag {};

}
