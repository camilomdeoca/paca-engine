#include "opengl/Texture.hpp"

#include "utils/Assert.hpp"

#include <array>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <GL/glew.h>

GLenum typeToOpenGLType(Texture::Type type)
{
    switch (type) {
        case Texture::Type::texture2D: return GL_TEXTURE_2D;
        case Texture::Type::cubeMap: return GL_TEXTURE_CUBE_MAP;
    }
    ASSERT_MSG(false, "Invalid Texture Type!");
}

GLenum formatToOpenGLFormat(Texture::Format format)
{
    switch (format)
    {
        case Texture::Format::G8:    return GL_RED;
        case Texture::Format::GA8:   return GL_RG;
        case Texture::Format::RGB8:  return GL_RGB;
        case Texture::Format::RGBA8: return GL_RGBA;
        case Texture::Format::RGBA16F: return GL_RGBA;
        case Texture::Format::depth24stencil8: return GL_DEPTH_STENCIL;
        case Texture::Format::depth24: return GL_DEPTH_COMPONENT;
    }
    ASSERT_MSG(false, "Invalid Texture Format!");
}

GLenum formatToOpenGLInternalFormat(Texture::Format format)
{
    switch (format)
    {
        case Texture::Format::G8:    return GL_R8;
        case Texture::Format::GA8:   return GL_RG8;
        case Texture::Format::RGB8:  return GL_RGB8;
        case Texture::Format::RGBA8: return GL_RGBA8;
        case Texture::Format::RGBA16F: return GL_RGBA16F;
        case Texture::Format::depth24stencil8: return GL_DEPTH24_STENCIL8;
        case Texture::Format::depth24: return GL_DEPTH_COMPONENT24;
    }
    ASSERT_MSG(false, "Invalid Texture Format!");
}

Texture::Texture(const std::string &path)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data)
        ASSERT_MSG(false, "Failed to load image: {}!", path);

    INFO("{} ({}x{}) has {} channels.", path, width, height, channels);
    switch (channels)
    {
        case 1: m_format = Format::G8; break;
        case 2: m_format = Format::GA8; break;
        case 3: m_format = Format::RGB8; break;
        case 4: m_format = Format::RGBA8; break;
    }

    create(data, width, height, m_format);

    stbi_image_free(data);
}

Texture::Texture(const uint8_t *data, uint32_t width, uint32_t height, Format format)
    : m_format(format)
{
    create(data, width, height, format);
}

Texture::Texture(std::array<const uint8_t*, 6> facesData, uint32_t width, uint32_t height, Format format)
{
    createCubeMap(facesData, width, height, format);
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

void Texture::create(const uint8_t *data, uint32_t width, uint32_t height, Format format)
{
    m_width = width;
    m_height = height;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, formatToOpenGLInternalFormat(format), m_width, m_height);
    ASSERT(glGetError() == 0);

    setInterpolate(true);
    setRepeat(true);

    if (data)
        glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, formatToOpenGLFormat(format), GL_UNSIGNED_BYTE, data);
    ASSERT(glGetError() == 0);
}

void Texture::createCubeMap(std::array<const uint8_t*, 6> facesData, uint32_t width, uint32_t height, Format format)
{
    m_width = width;
    m_height = height;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_id);
    glTextureStorage2D(m_id, 1, formatToOpenGLInternalFormat(format), m_width, m_height);

    setInterpolate(true);
    setRepeat(false);

    for (unsigned int i = 0; i < facesData.size(); i++)
    {
        glTextureSubImage3D(m_id, 0, 0, 0, i, m_width, m_height, 1, formatToOpenGLFormat(format), GL_UNSIGNED_BYTE, facesData[i]);
    }
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
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, value ? GL_LINEAR : GL_NEAREST);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, value ? GL_LINEAR : GL_NEAREST);
}

void Texture::setRepeat(bool value)
{
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, value ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, value ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, value ? GL_REPEAT : GL_CLAMP_TO_EDGE);
}

void Texture::setBorderColor(const glm::vec4 color)
{
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTextureParameterfv(m_id, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(color));
}

