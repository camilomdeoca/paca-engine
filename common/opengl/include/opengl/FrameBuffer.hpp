#pragma once

#include "opengl/Texture.hpp"

#include <cstdint>
#include <vector>

// TODO: add clear(vec3 color = {0, 0, 0}) method

struct FrameBufferParameters {
    uint32_t width = 0, height = 0;
    Texture &&depthTextureAttachment;
    std::vector<Texture> &&colorTextureAttachments;
};

class FrameBuffer {
public:
    FrameBuffer();
    FrameBuffer(FrameBufferParameters &&parameters);
    ~FrameBuffer();

    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    FrameBuffer(FrameBuffer &&source);
    FrameBuffer& operator=(FrameBuffer&& source);

    void init(FrameBufferParameters &&parameters);
    void shutdown();

    std::vector<Texture> &getColorAttachments() { return m_colorAttachments; }
    const std::vector<Texture> &getColorAttachments() const { return m_colorAttachments; }
    Texture &getDepthAttachment() { return m_depthAttachment; }
    const Texture &getDepthAttachment() const { return m_depthAttachment; }

    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }

    static void copy(const FrameBuffer &from, const FrameBuffer &to); // TODO: Specify what attachment is copied
    
    static FrameBuffer &getDefault() { return defaultFramebuffer; };

    void bind() const;
    void unbind() const;

private:
    // This framebuffer is used only to create the defaultFramebuffer object do not call
    FrameBuffer(uint32_t id) : m_id(id) {}
    static FrameBuffer defaultFramebuffer;

    uint32_t m_id;
    uint32_t m_width, m_height;
    std::vector<Texture> m_colorAttachments;
    Texture m_depthAttachment;
};
