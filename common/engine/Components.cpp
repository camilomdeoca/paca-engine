#include "engine/Components.hpp"
#include "utils/Assert.hpp"

namespace engine::components {

DirectionalLightShadowMap::DirectionalLightShadowMap(
    uint32_t shadowMapSize,
    const std::vector<float> &viewFrustumSplits)
{
    this->shadowMapSize = shadowMapSize;

    float prevSplit = 0.0f;
    levelCount = viewFrustumSplits.size();
    ASSERT(levelCount <= viewFrustumSplits.size());

    INFO("Shadow map levels limits:");
    for (unsigned int i = 0; i < viewFrustumSplits.size(); i++)
    {
        levels[i].near = prevSplit;
        levels[i].far = viewFrustumSplits[i];
        prevSplit = viewFrustumSplits[i];
        levels[i].cutoffDistance = viewFrustumSplits[i];
        INFO("\t Level {}: {} {}", i, levels[i].near, levels[i].far);
    }

    uint32_t width = shadowMapSize;
    uint32_t height = shadowMapSize * levelCount;

    Texture depthTexture(Texture::Specification{
        .width = width,
        .height = height,
        .format = Texture::Format::depth24,
    });
    depthTexture.setBorderColor({1.0f, 1.0f, 1.0f, 1.0f});

    shadowMapAtlasFramebuffer.init(FrameBufferParameters {
        .width = width,
        .height = height,
        .depthTextureAttachment = std::move(depthTexture),
        .colorTextureAttachments = {},
    });
}

DirectionalLightShadowMap::DirectionalLightShadowMap(DirectionalLightShadowMap &&source)
    : shadowMapSize(source.shadowMapSize),
      levelCount(source.levelCount),
      shadowMapAtlasFramebuffer(std::move(source.shadowMapAtlasFramebuffer))
{
    for(int i = 0; i < levelCount; i++)
    {
        levels[i] = source.levels[i];
    }
}

DirectionalLightShadowMap &DirectionalLightShadowMap::operator=(DirectionalLightShadowMap &&source)
{
    shadowMapSize = source.shadowMapSize;
    levelCount = source.levelCount;
    shadowMapAtlasFramebuffer = std::move(source.shadowMapAtlasFramebuffer);
    for(int i = 0; i < levelCount; i++)
    {
        levels[i] = source.levels[i];
    }
    return *this;
}

} // namespace engine::components

