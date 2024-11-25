#pragma once

#include "opengl/FrameBuffer.hpp"
#include <glm/glm.hpp>

constexpr size_t MAX_DIRECTIONAL_LIGHT_SHADOW_MAP_LEVELS = 5;

namespace engine::components {

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
    
    uint32_t shadowMapSize;
    ShadowMapLevel levels[MAX_DIRECTIONAL_LIGHT_SHADOW_MAP_LEVELS];
    uint8_t levelCount;
    FrameBuffer shadowMapAtlasFramebuffer;
};

}
