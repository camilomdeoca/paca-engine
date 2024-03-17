#include "FrameBuffer.hpp"

#include "opengl/Texture.hpp"

#include <GL/glew.h>
#include <cstdlib>
#include <memory>

FrameBuffer::FrameBuffer(FrameBufferParameters parameters)
{
    glGenFramebuffers(1, &m_id);
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);

    // Create textures for attaching to the framebuffer
    for (Texture::Format format : parameters.textureAttachmentFormats)
    {
        if (format != Texture::Format::depth24stencil8)
        {
            std::shared_ptr<Texture> &texture =
                m_colorAttachments.emplace_back(std::make_shared<Texture>(parameters.width, parameters.height, format));
            texture->setRepeat(false);
        }
        else
            m_depthAttachment = std::make_shared<Texture>(parameters.width, parameters.height, format);
    }

    // Attach all color textures
    for (unsigned int i = 0; i < m_colorAttachments.size(); i++)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_colorAttachments[i]->getId(), 0);

    // Attach depth stencil buffer
    if (m_depthAttachment)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_depthAttachment->getId(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        exit(1); // Error :C

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &m_id);
}

void FrameBuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void FrameBuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
