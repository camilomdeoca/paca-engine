#include "PreviewRenderer.hpp"
#include "engine/Loader.hpp"

#include <memory>
#include <opengl/FrameBuffer.hpp>
#include <opengl/gl.hpp>

#include <engine/PerspectiveCamera.hpp>
#include <engine/AxisAlignedBoundingBox.hpp>

void PreviewRenderer::init()
{
    std::list<ShaderCompileTimeParameter> staticMeshShaderParams;
    m_staticMeshShader = std::make_shared<Shader>(
        "assets/shaders/forwardStaticMeshVertex.glsl",
        "assets/shaders/forwardStaticMeshFragment.glsl",
        staticMeshShaderParams);

    m_cubeLinesShader = std::make_shared<Shader>(
        "assets/shaders/renderCubeLinesVertex.glsl",
        "assets/shaders/renderCubeLinesFragment.glsl");
    m_cubeVertexArrayForLines = std::make_shared<VertexArray>();
    float cubeVerticesForLines[] = {
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    };
    uint32_t cubeIndicesForLines[] = {
        // Front face
        0, 1, 1, 2, 2, 3, 3, 0,

        // Conections to back face
        0, 4, 1, 5, 2, 6, 3, 7,

        // Back face
        4, 5, 5, 6, 6, 7, 7, 4,
    };

    std::shared_ptr<VertexBuffer> cubeVertexBufferForLines
        = std::make_shared<VertexBuffer>(
            reinterpret_cast<uint8_t*>(cubeVerticesForLines),
            sizeof(cubeVerticesForLines)
        );
    cubeVertexBufferForLines->setLayout({ {ShaderDataType::float3, "a_position"} });
    m_cubeVertexArrayForLines->addVertexBuffer(cubeVertexBufferForLines);

    std::shared_ptr<IndexBuffer> cubeIndexBufferForLines
        = std::make_shared<IndexBuffer>(
            cubeIndicesForLines,
            sizeof(cubeIndicesForLines) / sizeof(uint32_t)
        );
    m_cubeVertexArrayForLines->setIndexBuffer(cubeIndexBufferForLines);

    auto staticMeshAssetData
        = engine::loaders::load<paca::fileformats::StaticMesh>("assets/meshes/icosphere/icosphere.gltf");
    m_sphereVertexArray = std::make_shared<VertexArray>();
    std::shared_ptr<VertexBuffer> sphereVertexBuffer
        = std::make_shared<VertexBuffer>(
            staticMeshAssetData->vertices.data(),
            staticMeshAssetData->vertices.size() * sizeof(staticMeshAssetData->vertices[0]));

    sphereVertexBuffer->setLayout({
        {ShaderDataType::float3, "a_position"},
        {ShaderDataType::float3, "a_normal"},
        {ShaderDataType::float3, "a_tangent"},
        {ShaderDataType::float2, "a_uvCoords"}
    });

    m_sphereVertexArray->addVertexBuffer(sphereVertexBuffer);

    std::shared_ptr<IndexBuffer> sphereIndexBuffer
        = std::make_shared<IndexBuffer>(
            staticMeshAssetData->indices.data(),
            staticMeshAssetData->indices.size());

    m_sphereVertexArray->setIndexBuffer(sphereIndexBuffer);

}

std::string textureTypeToUniformName(MaterialTextureType::Type type)
{
    switch (type) {
        case MaterialTextureType::diffuse: return "u_diffuseMap";
        case MaterialTextureType::specular: return "u_specularMap";
        case MaterialTextureType::normal: return "u_normalMap";
        case MaterialTextureType::height: return "u_heightMap";
        default: break;
    }

    ASSERT_MSG(false, "Invalid Material Texture Type!");
}

std::string textureTypeToHasTextureUniformName(MaterialTextureType::Type type)
{
    switch (type) {
        case MaterialTextureType::diffuse: return "u_hasDiffuse";
        case MaterialTextureType::specular: return "u_hasSpecular";
        case MaterialTextureType::normal: return "u_hasNormal";
        case MaterialTextureType::height: return "u_hasHeight";
        default: break;
    }

    ASSERT_MSG(false, "Invalid Material Texture Type!");
}

bool isFullyInside(const AxisAlignedBoundingBox &aabb, const glm::mat4 &frustum)
{
    (void)aabb.min;
    (void)aabb.max;
    const std::array<glm::vec4, 8> corners = {{
        {aabb.min.x, aabb.min.y, aabb.max.z, 1.0f},
        {aabb.max.x, aabb.min.y, aabb.max.z, 1.0f},
        {aabb.max.x, aabb.max.y, aabb.max.z, 1.0f},
        {aabb.min.x, aabb.max.y, aabb.max.z, 1.0f},
        {aabb.min.x, aabb.min.y, aabb.min.z, 1.0f},
        {aabb.max.x, aabb.min.y, aabb.min.z, 1.0f},
        {aabb.max.x, aabb.max.y, aabb.min.z, 1.0f},
        {aabb.min.x, aabb.max.y, aabb.min.z, 1.0f},
    }};

    for (const glm::vec4 &corner : corners)
    {
        const glm::vec4 projectedCornerVec4 = frustum * corner;
        const glm::vec3 projectedCorner = glm::vec3(projectedCornerVec4) / projectedCornerVec4.w;
        for (glm::length_t i = 0; i < 3; i++)
        {
            if (projectedCorner.x > 1.0f || projectedCorner.x < -1.0f) return false;
            if (projectedCorner.y > 1.0f || projectedCorner.y < -1.0f) return false;
            if (projectedCorner.z > 1.0f || projectedCorner.z < 0.0f) return false;
        }
    }
    return true;
}

void PreviewRenderer::drawPreviewToTexture(
    const StaticMesh *mesh,
    const Material *material,
    Texture &texture,
    const AssetManager &assetManager)
{
    PerspectiveCamera camera(1.0f, 90.0f);
    camera.setPosition({1.0f, 1.0f, 1.0f});
    camera.setRotation({-45.0f, -135.0f, 0.0f});
    std::vector<Texture> colorAttachments(1);
    colorAttachments[0] = std::move(texture);

    FrameBuffer framebuffer;
    framebuffer.init({
        .width = texture.getWidth(),
        .height = texture.getHeight(),
        .depthTextureAttachment = Texture(Texture::Specification{
            .width = texture.getWidth(),
            .height = texture.getHeight(),
            .format = Texture::Format::depth24,
        }),
        .colorTextureAttachments = std::move(colorAttachments),
    });
    framebuffer.bind();
    GL::setClearColor({1.0f, 1.0f, 1.0f, 0.0f});
    GL::clear();

    float scale = 1.0f;


    glm::vec3 aabbCenter;
    if (mesh)
    {
        aabbCenter = (mesh->getAABB().max + mesh->getAABB().min) * 0.5f;
        auto matrixFromScale = [&camera, &aabbCenter](float scale) {
            return camera.getViewProjectionMatrix() * glm::translate(glm::scale(glm::mat4(1.0f), {scale, scale, scale}), -aabbCenter);
        };

        if (isFullyInside(mesh->getAABB(), matrixFromScale(scale)))
        {
            while (isFullyInside(mesh->getAABB(), matrixFromScale(scale * 1.1f)))
            {
                scale *= 1.1f;
            }
        }
        else
        {
            while (!isFullyInside(mesh->getAABB(), matrixFromScale(scale * 0.9f)))
            {
                scale *= 0.9f;
            }
        }
    }
    else
    {
        aabbCenter = {0.0f, 0.0f, 0.0f};
        camera.setRotation({-35.0f, -135.0f, 0.0f});
    }


    m_staticMeshShader->bind();
    m_staticMeshShader->setUniform(camera.getProjectionMatrix(), "u_projectionMatrix");
    m_staticMeshShader->setUniform(
        camera.getViewMatrix() * glm::translate(glm::scale(glm::mat4(1.0f), {scale, scale, scale}), -aabbCenter),
        "u_viewModelMatrix");
    m_staticMeshShader->setUniform(
        0.05f,
        "u_parallaxScale");
    GL::setDepthTest(true);

    int slot = 0;
    if (material)
    {
        for (MaterialTextureType::Type i : {
            MaterialTextureType::diffuse,
            MaterialTextureType::specular,
            MaterialTextureType::normal
        }) {
            // Set uniform telling the shader if a texture of the type was provided
            m_staticMeshShader->setUniform(
                    material->getTextureIds(i).empty() ? 0 : 1,
                    "{}",
                    textureTypeToHasTextureUniformName(i));
            unsigned int indexOfTextureOfType = 0;
            for (const TextureId &textureId : material->getTextureIds(i))
            {
                const Texture *texture = assetManager.get(textureId);
                if (texture)
                {
                    texture->bind(slot);
                    m_staticMeshShader->setUniform(slot, "{}{}", textureTypeToUniformName(i), indexOfTextureOfType);
                    slot++, indexOfTextureOfType++;
                }
            }
        }
    }
    else
    {
        // When the mesh does not have a material set every has_{texture_type} uniform to false
        for (MaterialTextureType::Type i : {
            MaterialTextureType::diffuse,
            MaterialTextureType::specular,
            MaterialTextureType::normal
        }) {
            m_staticMeshShader->setUniform(0, "{}", textureTypeToHasTextureUniformName(i));
        }
    }

    // Directional Light
    glm::vec3 lightDirection = glm::normalize(glm::vec3(-2.0f, -1.0f, -0.5f));
    m_staticMeshShader->setUniform(glm::mat3(camera.getViewMatrix()) * lightDirection, "u_directionalLights[0].directionInViewSpace");
    m_staticMeshShader->setUniform(glm::vec3(1.0f), "u_directionalLights[0].color");
    m_staticMeshShader->setUniform(0.5f, "u_directionalLights[0].intensity");
    m_staticMeshShader->setUniform(1, "u_numOfDirectionalLights");

    m_staticMeshShader->setUniform(0, "u_numOfPointLights");

    if (mesh)
    {
        mesh->getVertexArray().bind();
        GL::viewport(framebuffer.getWidth(), framebuffer.getHeight());
        GL::drawIndexed(mesh->getVertexArray());
    }
    else
    {
        m_sphereVertexArray->bind();
        GL::viewport(framebuffer.getWidth(), framebuffer.getHeight());
        GL::drawIndexed(*m_sphereVertexArray);
    }

    if constexpr (false)
    {
        m_cubeLinesShader->bind();
        glm::mat4 matrix
            = camera.getViewProjectionMatrix()
            * glm::translate(glm::scale(glm::mat4(1.0f), {scale, scale, scale}), -aabbCenter)
            * glm::translate(glm::mat4(1.0f), mesh->getAABB().min)
            * glm::scale(glm::mat4(1.0f), mesh->getAABB().max - mesh->getAABB().min);
        m_cubeLinesShader->setUniform(matrix, "u_projectionView");
        GL::drawLines(*m_cubeVertexArrayForLines);
    }

    texture = std::move(framebuffer.getColorAttachments()[0]);
}
