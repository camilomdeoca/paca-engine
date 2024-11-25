#include "engine/Renderer2D.hpp"

#include "opengl/VertexArray.hpp"
#include "opengl/Shader.hpp"
#include "opengl/gl.hpp"
#include "opengl/Texture.hpp"
#include "opengl/VertexBuffer.hpp"

#include <codecvt>
#include <glm/ext/matrix_transform.hpp>
#include <locale>
#include <memory>
#include <unordered_map>
#include <fontatlas/fontatlas.hpp>

struct QuadVertex {
    glm::vec3 position;
    glm::vec2 uv;
};

static struct {
    std::shared_ptr<VertexArray> quadsVertexArray;
    std::shared_ptr<VertexBuffer> quadsVertexBuffer;
    std::shared_ptr<Shader> quadsShader;

    std::shared_ptr<Texture> whiteTexture;

    const glm::vec4 squareVertices[4] = {
        {0.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f}
    };

    const glm::vec2 squareUVCoords[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };
} s_data;

void Renderer2D::init()
{
    s_data.quadsVertexArray = std::make_shared<VertexArray>();

    s_data.quadsVertexBuffer = std::make_shared<VertexBuffer>(4 * sizeof(QuadVertex));
    s_data.quadsVertexBuffer->setLayout({
        {ShaderDataType::float3, "a_position"},
        {ShaderDataType::float2, "a_uv"}
    });

    s_data.quadsVertexArray->addVertexBuffer(s_data.quadsVertexBuffer);
    
    uint32_t squareIndices[] = { 0, 1, 2, 2, 3, 0 };
    std::shared_ptr<IndexBuffer> quadIndexBuffer;
    quadIndexBuffer = std::make_shared<IndexBuffer>(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    s_data.quadsVertexArray->setIndexBuffer(quadIndexBuffer);

    unsigned char whitePixel[] = { 255, 255, 255, 255 };

    s_data.whiteTexture = std::make_shared<Texture>(whitePixel, 1, 1, Texture::Format::RGBA8);

    s_data.quadsShader = std::make_shared<Shader>("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
}

void Renderer2D::beginScene(const Camera &camera)
{
    s_data.quadsShader->bind();
    s_data.quadsShader->setMat4("u_viewProjection", camera.getViewProjectionMatrix());
    GL::setBlending(true);
}

void Renderer2D::endScene()
{}

void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    drawQuad(position, size, *s_data.whiteTexture, color);
}

void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const Texture &texture, const glm::vec4 &color)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
        * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
    
    QuadVertex quadVertex[4];
    for (size_t i = 0; i < 4; i++)
    {
        quadVertex[i].position = transform * s_data.squareVertices[i];
        quadVertex[i].uv = s_data.squareUVCoords[i];
    }

    s_data.quadsVertexBuffer->setData(quadVertex, 4 * sizeof(QuadVertex));

    s_data.quadsShader->bind();
    s_data.quadsShader->setFloat4("u_color", color);

    texture.bind(0);
    s_data.quadsShader->setInt("u_texture", 0);

    s_data.quadsVertexArray->bind();
    GL::drawIndexed(*s_data.quadsVertexArray);
}


void Renderer2D::drawString(const glm::vec3 &position, const std::string &text, Font &font, const glm::vec4 &color, float size)
{
    std::u32string u32text = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(text);
    glm::vec3 currentGlyphPos = position;

    float w = font.getTexture()->getWidth();
    float h = font.getTexture()->getHeight();

    const std::unordered_map<uint32_t, fontatlas::glyph_data> &glyphsData = font.getGlyphsData();

    s_data.quadsShader->bind();
    s_data.quadsShader->setFloat4("u_color", color);

    font.getTexture()->bind(0);
    s_data.quadsShader->setInt("u_texture", 0);

    for (const char32_t &u32char : u32text) {
        std::unordered_map<uint32_t, fontatlas::glyph_data>::const_iterator it = glyphsData.find(u32char);
        if (it == glyphsData.end()) {
            continue;
        }

        fontatlas::glyph_data glyphData = it->second;

        if (glyphData.size.w != 0 && glyphData.size.h != 0) {
            float sizeX    = glyphData.size.w;
            float sizeY    = glyphData.size.h;

            float textureX      = glyphData.texture_coords.x/w;
            float textureY      = (h - glyphData.texture_coords.y - glyphData.size.h)/h; // Because we invert textures vertically on import
            float textureSizeX  = glyphData.size.w/w;
            float textureSizeY  = glyphData.size.h/h;

            glm::vec3 offset = { size * glyphData.offset.x, size * (glyphData.offset.y -  static_cast<int>(glyphData.size.h)), 0.0f };

            QuadVertex quadVertex[4];

            glm::vec3 relativeSize[4] = {
                { 0.0f,  0.0f, 0.0f},
                {sizeX,  0.0f, 0.0f},
                {sizeX, sizeY, 0.0f},
                { 0.0f, sizeY, 0.0f}
            };

            glm::vec2 uvCoords[] = {
                {               textureX,                textureY},
                {textureX + textureSizeX,                textureY},
                {textureX + textureSizeX, textureY + textureSizeY},
                {               textureX, textureY + textureSizeY}
            };

            glm::vec3 glyphOrigin = currentGlyphPos + offset;
            for (size_t i = 0; i < 4; i++)
            {
                quadVertex[i].position = glyphOrigin + relativeSize[i] * size;
                quadVertex[i].uv = uvCoords[i];
            }

            s_data.quadsVertexBuffer->setData(quadVertex, 4 * sizeof(QuadVertex));
            s_data.quadsVertexArray->bind();
            GL::drawIndexed(s_data.quadsVertexArray);
        }
        currentGlyphPos.x += size * (glyphData.advance.x);
        currentGlyphPos.y += size * (glyphData.advance.y);
    } 
}
