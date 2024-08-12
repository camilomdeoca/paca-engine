#pragma once

#include "opengl/Texture.hpp"

#include <cstdint>
#include <memory>
#include <vector>

// TODO: add clear(vec3 color = {0, 0, 0}) method

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
    
    static const FrameBuffer &getDefault() { return defaultFramebuffer; };

    void bind();
    void unbind();

private:
    // This framebuffer is used only to create the defaultFramebuffer object do not call
    FrameBuffer(uint32_t id) {}
    static const FrameBuffer defaultFramebuffer;

    uint32_t m_id;
    uint32_t m_width, m_height;
    std::vector<std::shared_ptr<Texture>> m_colorAttachments;
    std::shared_ptr<Texture> m_depthAttachment;
};
