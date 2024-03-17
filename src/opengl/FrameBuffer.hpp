#pragma once

#include "opengl/Texture.hpp"

#include <cstdint>
#include <memory>
#include <vector>

struct FrameBufferParameters {
    uint32_t width = 0, height = 0;
    std::vector<Texture::Format> textureAttachmentFormats;
};

class FrameBuffer {
public:
    FrameBuffer(FrameBufferParameters parameters);
    ~FrameBuffer();

    const std::vector<std::shared_ptr<Texture>> &getColorAttachments() const { return m_colorAttachments; }
    std::shared_ptr<Texture> getDepthAttachment() { return m_depthAttachment; }

    void bind();
    void unbind();

private:
    uint32_t m_id;
    std::vector<std::shared_ptr<Texture>> m_colorAttachments;
    std::shared_ptr<Texture> m_depthAttachment;
};
