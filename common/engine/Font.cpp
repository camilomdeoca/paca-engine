#include "engine/Font.hpp"
#include <utility>

Font::Font(
    TextureId fontAtlasTextureId,
    unsigned int fontHeight,
    const std::vector<paca::fileformats::GlyphData> &glyphsData)
    : m_atlas(fontAtlasTextureId),
      m_fontHeight(fontHeight)
{
    for (const paca::fileformats::GlyphData &glyph : glyphsData)
    {
        m_glyphsData.emplace(std::make_pair(
            glyph.characterCode,
            GlyphData(
                glyph.textureCoords,
                glyph.size,
                glyph.advance,
                glyph.offset)));
    }
}

