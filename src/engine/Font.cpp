#include "Font.hpp"

#include "fontatlas/fontatlas.hpp"

#include <memory>

Font::Font(const std::string &atlasImagePath, const std::string &glyphsDataFile)
{
    m_texture = std::make_shared<Texture>(atlasImagePath);
    std::pair<fontatlas::atlas_header, std::unordered_map<uint32_t, fontatlas::glyph_data>> pair =
        fontatlas::read_glyph_data_file(glyphsDataFile);
    m_glyphs_data = pair.second;
    m_fontHeight = pair.first.font_height;
}

