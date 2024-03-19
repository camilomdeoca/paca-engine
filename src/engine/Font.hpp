#pragma once

#include "fontatlas/fontatlas.hpp"
#include "opengl/Texture.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

class Font {
public:
    Font(const std::string &atlasImagePath, const std::string &glyphsDataFile);
    const std::shared_ptr<Texture> &getTexture() { return m_texture; }
    const std::unordered_map<uint32_t, fontatlas::glyph_data> &getGlyphsData() { return m_glyphs_data; }
    unsigned int getHeight() { return m_fontHeight; }

private:
    std::shared_ptr<Texture> m_texture;
    std::unordered_map<uint32_t, fontatlas::glyph_data> m_glyphs_data;
    unsigned int m_fontHeight;
};
