#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <array>

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

    struct Specification {
        const uint8_t *data = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        Format format = Format::RGBA8;
        uint32_t mipmapLevels = 1; // 1 to disable
        bool autoGenerateMipmapLevels = false;
        bool linearMinification = true; // if set to true it interpolates on minification
        bool linearMagnification = true; // if set to true it interpolates on magnification
        bool interpolateBetweenMipmapLevels = false;
    };
    struct CubeMapSpecification {
        std::array<const uint8_t*, 6> facesData = {};
        uint32_t width = 0;
        uint32_t height = 0;
        Format format = Format::RGBA8;
        uint32_t mipmapLevels = 1; // 1 to disable
        bool autoGenerateMipmapLevels = false;
        bool linearMinification = true; // if set to true it interpolates on minification
        bool linearMagnification = true; // if set to true it interpolates on magnification
        bool interpolateBetweenMipmapLevels = false;
    };

    Texture(const Specification &specification);
    Texture(const CubeMapSpecification &specification);
    //Texture(const std::string &path);
    //Texture(const uint8_t *data, uint32_t width, uint32_t height, Format format);
    //Texture(std::array<const uint8_t*, 6> facesData, uint32_t width, uint32_t height, Format format);
    //Texture(uint32_t width, uint32_t height, Format format);
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
    void create(const Specification &specification);
    // The data parameter are the pointers to each face data
    void createCubeMap(const CubeMapSpecification &specification);

    uint32_t m_id;
    uint32_t m_width, m_height;
    Format m_format;
    Type m_type;
};
