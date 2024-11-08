#include "FontConverter.hpp"

#include "ResourceFileFormats.hpp"

#include <utils/Log.hpp>

#include <codecvt>
#include <ft2build.h>
#include <string>
#include FT_FREETYPE_H

constexpr unsigned int atlasWidth = 256;
constexpr unsigned int atlasHeight = 256;
constexpr unsigned int FONT_HEIGHT = 12;
const std::string charSet = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRST"
                            "UVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ÁÉÍÓÚáéíóúñ";

paca::fileformats::AssetPack fontToPacaFormat(const std::string &fontPath, const std::string &outName)
{
    paca::fileformats::AssetPack pack;
    pack.fonts.emplace_back();
    pack.textures.emplace_back();
    paca::fileformats::Font &font = pack.fonts.back();
    paca::fileformats::Texture &fontAtlas = pack.textures.back();

    unsigned int fontHeight = FONT_HEIGHT;

    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        ERROR("Error initializing FreeType library.");

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        ERROR("Error loading font.");

    if (FT_Set_Pixel_Sizes(face, 0, fontHeight))
        ERROR("Error setting font size.");

    fontAtlas.name = outName;
    fontAtlas.width = atlasWidth;
    fontAtlas.height = atlasHeight;
    fontAtlas.channels = 1;
    fontAtlas.pixelData.resize(atlasWidth * atlasHeight, 0);

    font.name = outName;
    font.fontHeight = fontHeight;
    font.atlasTextureName = fontAtlas.name;

    std::u32string u32charSet = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(charSet);

    int padding = 1;
    int col = padding, row = 0;
    for (size_t i = 0; i < u32charSet.size(); i++) {
        FT_ULong charCode = u32charSet[i];
        
        FT_UInt glyphIndex = FT_Get_Char_Index(face, charCode);
        if (glyphIndex == 0)
        {
            ERROR("Invalid char index: {}", charCode);
            continue;
        }

        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
        {
            ERROR("Error loading glyph: {}", charCode);
            continue;
        }

        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
        {
            ERROR("Error rendering glyph: {}", charCode);
            continue;
        }

        if (col + face->glyph->bitmap.width + padding >= atlasWidth) {
            col = padding;
            row += fontHeight + padding;
        }

        if ((face->size->metrics.ascender - face->size->metrics.descender) >> 6 > fontHeight) {
            fontHeight = (face->size->metrics.ascender - face->size->metrics.descender) >> 6;
        }

        for (unsigned int y = 0; y < face->glyph->bitmap.rows; y++) {
            for (unsigned int x = 0; x < face->glyph->bitmap.width; x++) {
                char val = face->glyph->bitmap.buffer[y * face->glyph->bitmap.width + x];
                fontAtlas.pixelData[(row + y)*atlasWidth + col+x] = val;
            }
        }

        font.glyphs.emplace_back(paca::fileformats::GlyphData{
            .characterCode = static_cast<uint32_t>(charCode),
            .textureCoords = {col, row},
            .size = {static_cast<uint16_t>(face->glyph->bitmap.width), static_cast<uint16_t>(face->glyph->bitmap.rows)},
            .advance = {static_cast<int16_t>(face->glyph->advance.x), static_cast<int16_t>(face->glyph->advance.y)},
            .offset = {static_cast<int16_t>(face->glyph->bitmap_left), static_cast<int16_t>(face->glyph->bitmap_top)}
        });

        col += face->glyph->bitmap.width + padding;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return pack;
}
