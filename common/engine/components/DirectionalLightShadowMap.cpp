#include "engine/components/DirectionalLightShadowMap.hpp"
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

    FrameBufferParameters shadowDepthMapBufferParams;
    shadowDepthMapBufferParams.width = shadowMapSize;
    shadowDepthMapBufferParams.height = shadowMapSize * levelCount;
    shadowDepthMapBufferParams.textureAttachmentFormats = { Texture::Format::depth24 };
    shadowMapAtlasFramebuffer.init(shadowDepthMapBufferParams);
    shadowMapAtlasFramebuffer.getDepthAttachment()->setBorderColor({1.0f, 1.0f, 1.0f, 1.0f});
}

} // namespace engine::components

