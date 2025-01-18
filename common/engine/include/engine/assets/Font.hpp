#pragma once

#include "ResourceFileFormats.hpp"
#include "engine/IdTypes.hpp"

#include <cstdint>
#include <unordered_map>

struct GlyphData {
    glm::vec<2, int32_t> textureCoords;
    glm::vec<2, uint16_t> size;
    glm::vec<2, int16_t> advance;
    glm::vec<2, int16_t> offset;
};

class Font {
public:
    Font(
        TextureId fontAtlasTextureId,
        unsigned int fontHeight,
        const std::vector<paca::fileformats::GlyphData> &glyphsData);

    TextureId getTextureId() { return m_atlas; }
    const std::unordered_map<uint32_t, GlyphData> &getGlyphsData() { return m_glyphsData; }
    unsigned int getHeight() { return m_fontHeight; }

private:
    TextureId m_atlas;
    std::unordered_map<uint32_t, GlyphData> m_glyphsData;
    unsigned int m_fontHeight;
};
