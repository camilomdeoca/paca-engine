#include "Renderer2D.hpp"
#include "../opengl/VertexArray.hpp"
#include "../opengl/Shader.hpp"
#include "../opengl/gl.hpp"
#include "opengl/Texture.hpp"
#include "opengl/VertexBuffer.hpp"

#include <codecvt>
#include <glm/ext/matrix_transform.hpp>
#include <locale>
#include <memory>

#include "glyph_data_deja_vu_sans.h"

struct QuadVertex {
    glm::vec3 position;
    glm::vec2 uv;
};

static struct {
    std::shared_ptr<VertexArray> quadsVertexArray;
    std::shared_ptr<VertexBuffer> quadsVertexBuffer;
    std::shared_ptr<Shader> quadsShader;

    std::shared_ptr<Texture> whiteTexture;
    std::shared_ptr<Texture> fontAtlas;

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

    s_data.whiteTexture = std::make_shared<Texture>(whitePixel, 1, 1);
    s_data.fontAtlas = std::make_shared<Texture>("assets/fonts/DejaVuSansFontAtlas.png");
    s_data.fontAtlas->setInterpolate(false);

    s_data.quadsShader = std::make_shared<Shader>("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");
}

void Renderer2D::beginScene(const OrthoCamera &camera)
{
    s_data.quadsShader->bind();
    s_data.quadsShader->setMat4("u_viewProjection", camera.getViewProjectionMatrix());
}

void Renderer2D::endScene()
{}

void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
{
    drawQuad(position, size, s_data.whiteTexture, color);
}

void Renderer2D::drawQuad(const glm::vec3 &position, const glm::vec2 &size, std::shared_ptr<Texture> texture, const glm::vec4 &color)
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

    texture->bind(0);
    s_data.quadsShader->setInt("u_texture", 0);

    s_data.quadsVertexArray->bind();
    GL::drawIndexed(s_data.quadsVertexArray);
}


void Renderer2D::drawString(const glm::vec3 &position, const std::string &text, const glm::vec4 &color)
{
    std::u32string u32text = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>().from_bytes(text);
    glm::vec3 currentGlyphPos = position;

    float w = s_data.fontAtlas->getWidth();
    float h = s_data.fontAtlas->getHeight();
    float scale = 1.0f;

    for (const char32_t &u32char : u32text) {
        const std::unordered_map<char32_t, Glyph>::iterator it = fontData.find(u32char);
        if (it == fontData.end()) {
            continue;
        }

        Glyph glyphData = it->second;

        if (glyphData.size.x != 0 && glyphData.size.y != 0) {
            float sizeX    = glyphData.size.x;
            float sizeY    = glyphData.size.y;

            float textureX      = glyphData.textureCoords.x/w;
            float textureY      = (h - glyphData.textureCoords.y - glyphData.size.y)/h; // Because we invert textures vertically on import
            float textureSizeX  = glyphData.size.x/w;
            float textureSizeY  = glyphData.size.y/h;

            glm::vec3 offset = { scale * glyphData.offset.x, scale * (glyphData.offset.y -  static_cast<int>(glyphData.size.y)), 0.0f };

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
                quadVertex[i].position = glyphOrigin + relativeSize[i] * scale;
                quadVertex[i].uv = uvCoords[i];
            }

            s_data.quadsVertexBuffer->setData(quadVertex, 4 * sizeof(QuadVertex));

            s_data.quadsShader->bind();
            s_data.quadsShader->setFloat4("u_color", color);

            s_data.fontAtlas->bind(0);
            s_data.quadsShader->setInt("u_texture", 0);

            s_data.quadsVertexArray->bind();
            GL::drawIndexed(s_data.quadsVertexArray);
        }
        currentGlyphPos.x += scale * (glyphData.advance.x);
        currentGlyphPos.y += scale * (glyphData.advance.y);
    } 
}
