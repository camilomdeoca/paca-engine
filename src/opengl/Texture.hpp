#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <string>

class Texture {
public:
    enum class Type {
        texture2D,
        cubeMap
    };

    enum class Format {
        G8,
        GA8,
        RGB8,
        RGBA8,
        RGBA16F,
        depth24stencil8,
        depth24
    };

    // Create 2D texture
    Texture(const std::string &path);

    // Create Cube Map
    Texture(const std::array<std::string, 6> paths);

    Texture(unsigned char *data, uint32_t width, uint32_t height, Format format);
    Texture(uint32_t width, uint32_t height, Format format);
    ~Texture();

    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }
    Format getFormat() const { return m_format; }

    void setData(void *data, uint32_t size);

    void bind(uint32_t slot = 0) const;

    void setInterpolate(bool value);
    void setRepeat(bool value);
    void setBorderColor(const glm::vec4 color);

    // Needed for the framebuffer class. Is there a better way without exposing the id?
    uint32_t getId() { return m_id; }

private:
    void create(unsigned char *data, uint32_t width, uint32_t height, Format format);
    // The data parameter are the pointers to each face data
    void createCubeMap(std::array<unsigned char*, 6> facesData, uint32_t width, uint32_t height, Format format);

    uint32_t m_id;
    uint32_t m_width, m_height;
    Format m_format;
    Type m_type;
};
