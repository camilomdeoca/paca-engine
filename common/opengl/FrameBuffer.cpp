#include "opengl/FrameBuffer.hpp"

#include <utils/Assert.hpp>
#include "opengl/Texture.hpp"

#include <GL/glew.h>
#include <memory>
#include <vector>

FrameBuffer FrameBuffer::defaultFramebuffer(0);

GLenum depthBufferFormatToAttachment(Texture::Format format)
{
    switch (format) {
        case Texture::Format::depth24stencil8: return GL_DEPTH_STENCIL_ATTACHMENT;
        case Texture::Format::depth24: return GL_DEPTH_ATTACHMENT;
        default: ASSERT_MSG(false, "Invalid depth texture format");
    }
}

FrameBuffer::FrameBuffer()
    : m_id(0)
{}

FrameBuffer::FrameBuffer(FrameBufferParameters parameters)
{
    init(parameters);
}

void FrameBuffer::init(FrameBufferParameters parameters)
{
    m_width = parameters.width;
    m_height = parameters.height;

    glGenFramebuffers(1, &m_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);

    // Create textures for attaching to the framebuffer
    for (Texture::Format format : parameters.textureAttachmentFormats)
    {
        if (format != Texture::Format::depth24stencil8 && format != Texture::Format::depth24)
        {
            std::shared_ptr<Texture> &texture =
                m_colorAttachments.emplace_back(std::make_shared<Texture>(Texture::Specification{
                    .width = parameters.width,
                    .height = parameters.height,
                    .format = format,
                    .linearMinification = false,
                    .linearMagnification = false,
                    .interpolateBetweenMipmapLevels = false,
                }));
            texture->setRepeat(false);
        }
        else
        {
            m_depthAttachment = std::make_shared<Texture>(Texture::Specification{
                .width = parameters.width,
                .height = parameters.height,
                .format = format,
            });
        }
    }

    // Attach all color textures
    for (unsigned int i = 0; i < m_colorAttachments.size(); i++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorAttachments[i]->getId(), 0);
    }

    // Attach depth stencil buffer
    if (m_depthAttachment)
        glFramebufferTexture2D(GL_FRAMEBUFFER, depthBufferFormatToAttachment(m_depthAttachment->getFormat()), GL_TEXTURE_2D, m_depthAttachment->getId(), 0);

    // Set color buffers to draw to
    GLenum buffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4,
        GL_COLOR_ATTACHMENT5,
        GL_COLOR_ATTACHMENT6,
        GL_COLOR_ATTACHMENT7,
        GL_COLOR_ATTACHMENT8
    };
    ASSERT(m_colorAttachments.size() < sizeof(buffers) / sizeof(GLenum));
    if (!m_colorAttachments.empty())
        glDrawBuffers(m_colorAttachments.size(), buffers);
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    unsigned int error = glGetError();
    ASSERT_MSG(error == 0, "OpenGl Error: {}", error);

    ASSERT_MSG(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &m_id);
}

void FrameBuffer::shutdown()
{
    if (m_id) glDeleteFramebuffers(1, &m_id);
    m_depthAttachment.reset();
    m_colorAttachments.clear();
}

void FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::copy(const FrameBuffer &from, const FrameBuffer &to)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, from.m_id);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, to.m_id);
    glBlitFramebuffer(0, 0, from.m_width, from.m_height, 0, 0, to.m_width, to.m_height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}
