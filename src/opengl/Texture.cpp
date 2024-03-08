#include "Texture.hpp"

#include <cstdio>
#include <stb_image.h>
#include <GL/glew.h>

Texture::Texture(const std::string &path)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc *data = stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (!data) {
        fprintf(stderr, "Failed to load image!\n");
        exit(1);
    }

    create(data, width, height);

    stbi_image_free(data);
}

Texture::Texture(unsigned char *data, uint32_t width, uint32_t height)
{
    create(data, width, height);
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_id);
}

void Texture::create(unsigned char *data, uint32_t width, uint32_t height)
{
    m_width = width;
    m_height = height;

    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, GL_RGBA8, m_width, m_height);

    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Texture::bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_id);
}

