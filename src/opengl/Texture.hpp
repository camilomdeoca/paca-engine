#pragma once

#include <cstdint>
#include <string>

class Texture {
public:
    Texture(const std::string &path);

    // For now data in RGBA8 format
    Texture(unsigned char *data, uint32_t width, uint32_t height);
    ~Texture();

    uint32_t getWidth() const { return m_width; }
    uint32_t getHeight() const { return m_height; }

    void setData(void *data, uint32_t size);

    void bind(uint32_t slot = 0) const;

    void setInterpolate(bool value);

private:
    void create(unsigned char *data, uint32_t width, uint32_t height);

    uint32_t m_id;
    uint32_t m_width, m_height;
};
