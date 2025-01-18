#pragma once

#include <engine/AssetManager.hpp>
#include <engine/assets/StaticMesh.hpp>
#include <opengl/Shader.hpp>

class PreviewRenderer {
public:
    void init();
    void drawPreviewToTexture(
        const StaticMesh *mesh,
        const Material *material,
        Texture &texture,
        const AssetManager &assetManager);

private:
    std::shared_ptr<Shader> m_staticMeshShader; // Get this from the ResourceManager
                                                // so it isnt recreated with multiple renderers

    std::shared_ptr<Shader> m_cubeLinesShader;
    std::shared_ptr<VertexArray> m_cubeVertexArrayForLines;
    std::shared_ptr<VertexArray> m_sphereVertexArray;
};
