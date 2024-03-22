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

    static void copy(const FrameBuffer &from, const FrameBuffer &to); // TODO: Specify what attachment is copied

    void bind();
    void unbind();

private:
    uint32_t m_id;
    uint32_t m_width, m_height;
    std::vector<std::shared_ptr<Texture>> m_colorAttachments;
    std::shared_ptr<Texture> m_depthAttachment;
};
