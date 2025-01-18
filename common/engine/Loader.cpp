#include "engine/Loader.hpp"

#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <utils/Assert.hpp>

#include <cgltf.h>

#include <stb_image.h>

namespace engine::loaders {

void readMesh(const cgltf_mesh &mesh, paca::fileformats::StaticMesh &outMesh)
{ 
    outMesh.name = mesh.name;
    outMesh.materialId = 0;
    outMesh.id = 0;

    const cgltf_primitive &primitive = mesh.primitives[0];
    ASSERT_MSG(primitive.type == cgltf_primitive_type_triangles, "Only triangle meshes supported");
    ASSERT(primitive.attributes_count > 0);

    INFO("Attribute count {}", primitive.attributes_count);
    cgltf_size vertexCount = primitive.attributes[0].data->count;

    outMesh.vertices.resize(sizeof(paca::fileformats::StaticMesh::Vertex) * vertexCount);
    paca::fileformats::StaticMesh::Vertex *vertices
        = reinterpret_cast<paca::fileformats::StaticMesh::Vertex*>(outMesh.vertices.data());

    int positionAttributeIndex = -1;
    int normalAttributeIndex = -1;
    int textureAttributeIndex = -1;
    int tangentAttributeIndex = -1;
    
    for (cgltf_size i = 0; i < primitive.attributes_count; i++)
    {
        INFO("Attribute {}, count {}", primitive.attributes[i].name,
                primitive.attributes[i].data->count);

        cgltf_attribute_type type = primitive.attributes[i].type;
        switch (type) {
        case cgltf_attribute_type_position:
            positionAttributeIndex = i;
        case cgltf_attribute_type_normal:
            normalAttributeIndex = i;
            break;
        case cgltf_attribute_type_tangent:
            tangentAttributeIndex = i;
            break;
        case cgltf_attribute_type_texcoord:
            textureAttributeIndex = i;
            break;
        case cgltf_attribute_type_invalid:
        case cgltf_attribute_type_color:
        case cgltf_attribute_type_joints:
        case cgltf_attribute_type_weights:
        case cgltf_attribute_type_custom:
        case cgltf_attribute_type_max_enum:
            WARN("Attribute is unsupported");
            break;
        }
    }


    if (positionAttributeIndex == -1)
    {
        ASSERT_MSG(false, "Position Attribute is required");
        return;
    }

    ASSERT(primitive.attributes[positionAttributeIndex].type == cgltf_attribute_type_position);
    ASSERT(primitive.attributes[positionAttributeIndex].data->type == cgltf_type_vec3);
    ASSERT(primitive.attributes[normalAttributeIndex].data->type == cgltf_type_vec3);
    ASSERT(primitive.attributes[textureAttributeIndex].data->type == cgltf_type_vec2);
    ASSERT(primitive.attributes[tangentAttributeIndex].data->type == cgltf_type_vec4);

    glm::vec3 firstVertexPosition;
    cgltf_accessor_read_float(
        primitive.attributes[positionAttributeIndex].data, 0,
        glm::value_ptr(firstVertexPosition), 3);
    outMesh.aabb = {
        .min = firstVertexPosition,
        .max = firstVertexPosition,
    };
    for (cgltf_size i = 0; i < vertexCount; i++)
    {
        cgltf_accessor_read_float(
                primitive.attributes[positionAttributeIndex].data, i,
                glm::value_ptr(vertices[i].position), 3);

        // Calculate AABB
        for (glm::length_t j = 0; j < outMesh.aabb.min.length(); j++)
        {
            if (vertices[i].position[j] > outMesh.aabb.min[j])
                outMesh.aabb.min[j] = vertices[i].position[j];
            if (vertices[i].position[j] < outMesh.aabb.max[j])
                outMesh.aabb.max[j] = vertices[i].position[j];
        }

        if (normalAttributeIndex != -1)
        {
            cgltf_accessor_read_float(primitive.attributes[normalAttributeIndex].data, i,
                    glm::value_ptr(vertices[i].normal), 3);
        }

        if (textureAttributeIndex != -1)
        {
            cgltf_accessor_read_float(primitive.attributes[textureAttributeIndex].data, i,
                    glm::value_ptr(vertices[i].texture), 2);
        }

        if (tangentAttributeIndex != -1)
        {
            glm::vec4 tangent;
            cgltf_accessor_read_float(primitive.attributes[tangentAttributeIndex].data, i,
                    glm::value_ptr(tangent), 4);
            vertices[i].tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
        }
    }
    INFO(
        "AABB ({} {} {})_({} {} {})",
        outMesh.aabb.min.x,
        outMesh.aabb.min.y,
        outMesh.aabb.min.z,
        outMesh.aabb.max.x,
        outMesh.aabb.max.y,
        outMesh.aabb.max.z);

    outMesh.indices.resize(primitive.indices->count);
    for (cgltf_size i = 0; i < primitive.indices->count; i++)
    {
        outMesh.indices[i] = cgltf_accessor_read_index(primitive.indices, i);
    }
}

template<>
std::optional<paca::fileformats::StaticMesh> load<paca::fileformats::StaticMesh>(const char *path)
{
    cgltf_options options {cgltf_file_type_invalid, 0};
    cgltf_data *data = NULL;
    const cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result != cgltf_result_success)
    {
        ERROR("Error loading GLTF file: {}", path);
        return {};
    }

    const cgltf_result bufferLoadResult = cgltf_load_buffers(&options, data, path);

    if (bufferLoadResult != cgltf_result_success)
    {
        ERROR("Error loading GLTF file buffers for: {}", path);
        return {};
    }

    for (cgltf_size m = 0; m < data->meshes_count; m++)
    {
        INFO("\t{}", data->meshes[m].name);
    }

    paca::fileformats::StaticMesh mesh;
    readMesh(data->meshes[0], mesh);
    return mesh;
}

template<>
std::optional<paca::fileformats::Texture> load<paca::fileformats::Texture>(const char *path)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(0);
    stbi_uc *data = stbi_load(path, &width, &height, &channels, 0);

    if (!data)
    {
        ERROR("Failed to load image: {}!", path);
        return {};
    }

    paca::fileformats::Texture texture;
    texture.name = path;
    texture.id = 0;
    texture.width = width;
    texture.height = height;
    texture.channels = channels;
    texture.pixelData.resize(width * height * channels);
    memcpy(texture.pixelData.data(), data, width * height * channels);

    INFO("{} ({}x{}) has {} channels.", path, width, height, channels);

    stbi_image_free(data);

    return texture;
}

template<>
std::optional<paca::fileformats::CubeMap> load<paca::fileformats::CubeMap>(const char *path)
{
    const std::array<const char*, 6> facesNames = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    };

    stbi_set_flip_vertically_on_load(1);
    std::array<std::string, 6> filePaths;
    filePaths[0] = std::filesystem::path(path) / facesNames[0];
    int width, height, channels;

    int result = stbi_info(filePaths[0].c_str(), &width, &height, &channels);
    if (!result)
    {
        ERROR("Couldnt load image info: {}", filePaths[0]);
        return {};
    }

    for (unsigned int i = 1; i < facesNames.size(); i++)
    {
        filePaths[i] = std::filesystem::path(path) / facesNames[i];
        int newWidth, newHeight, newChannels;
        int result = stbi_info(filePaths[i].c_str(), &newWidth, &newHeight, &newChannels);

        if (!result)
        {
            ERROR("Couldnt load image info: {}", filePaths[0]);
            return {};
        }

        if (newWidth != width || newHeight != height || newChannels != channels)
        {
            ERROR("Cubemap image: {}, has different size", filePaths[i]);
            return {};
        }
    }

    paca::fileformats::CubeMap cubemap;
    cubemap.pixelData.resize(width*height*channels*6);

    for (unsigned int i = 0; i < facesNames.size(); i++)
    {
        stbi_set_flip_vertically_on_load(0);

        stbi_uc *data = stbi_load(filePaths[i].c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            ERROR("Failed to load image: {}!", filePaths[i].c_str()); 
            return {};
        }

        INFO("{} ({}x{}) has {} channels.", filePaths[i].c_str(), width, height, channels);

        memcpy(
            cubemap.pixelData.data() + width * height * channels * i,
            data,
            width * height * channels);

        stbi_image_free(data);
    }

    cubemap.width = width;
    cubemap.height = height;
    cubemap.channels = channels;
    cubemap.name = path;
    cubemap.id = 0;

    return cubemap;
}

} // namespace engine::loaders
