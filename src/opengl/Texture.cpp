#include "Texture.hpp"
#include "util/ImageLoader.hpp"

#include <cstdio>
#include <stb_image.h>
#include <GL/glew.h>
#include <vector>

GLenum formatToOpenGLFormat(Texture::Format format)
{
    switch (format)
    {
        case Texture::Format::G8:    return GL_RED;
        case Texture::Format::GA8:   return GL_RG;
        case Texture::Format::RGB8:  return GL_RGB;
        case Texture::Format::RGBA8: return GL_RGBA;
        case Texture::Format::depth24stencil8: return GL_DEPTH_STENCIL;
    }
}

GLenum formatToOpenGLInternalFormat(Texture::Format format)
{
    switch (format)
    {
        case Texture::Format::G8:    return GL_R8;
        case Texture::Format::GA8:   return GL_RG8;
        case Texture::Format::RGB8:  return GL_RGB8;
        case Texture::Format::RGBA8: return GL_RGBA8;
        case Texture::Format::depth24stencil8: return GL_DEPTH24_STENCIL8;
    }
}

Texture::Texture(const std::string &path)
{
    unsigned int width, height, channels;
    //stbi_set_flip_vertically_on_load(1);
    //stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    std::vector<unsigned char> data = ImageLoader::loadPNG(path, width, height, channels);

    if (data.empty())
    {
        fprintf(stderr, "Failed to load image!\n");
        exit(1);
    }
    Format format;
    printf("%s (%dx%d) has %d channels\n", path.c_str(), width, height, channels);
    switch (channels)
    {
        case 1: format = Format::G8; break;
        case 2: format = Format::GA8; break;
        case 3: format = Format::RGB8; break;
        case 4: format = Format::RGBA8; break;
    }

    create(data.data(), width, height, format);

    //stbi_image_free(data);
}

Texture::Texture(unsigned char *data, uint32_t width, uint32_t height, Format format)
    : m_format(format)
{
    create(data, width, height, format);
}

Texture::Texture(uint32_t width, uint32_t height, Format format)
    : m_format(format)
{
    create(nullptr, width, height, format);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

void Texture::create(unsigned char *data, uint32_t width, uint32_t height, Format format)
{
    m_width = width;
    m_height = height;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, formatToOpenGLInternalFormat(format), m_width, m_height);

    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    if (data)
        glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, formatToOpenGLFormat(format), GL_UNSIGNED_BYTE, data);
}

//void Texture::setData(void *data, uint32_t size)
//{
//    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, formatToOpenGLFormat(m_format), GL_UNSIGNED_BYTE, data);
//}

void Texture::bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_id);
}

void Texture::setInterpolate(bool value)
{
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, value ? GL_LINEAR : GL_NEAREST);
}

void Texture::setRepeat(bool value)
{
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, value ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, value ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, value ? GL_REPEAT : GL_CLAMP_TO_EDGE);
}

