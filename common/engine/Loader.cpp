#include "engine/Loader.hpp"

#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <limits>
#include <unordered_map>
#include <utils/Assert.hpp>

#include <cgltf.h>

#include <stb_image.h>

namespace engine::loaders {

template<typename MeshType>
void readMesh(const cgltf_mesh &mesh, MeshType &outMesh)
{
    constexpr bool isAnimated = requires (MeshType::Vertex vertex) {
        vertex.boneIDs;
        vertex.boneWeights;
    };

    outMesh.name = mesh.name;
    outMesh.id = 0;

    const cgltf_primitive &primitive = mesh.primitives[0];
    ASSERT_MSG(primitive.type == cgltf_primitive_type_triangles, "Only triangle meshes supported");
    ASSERT(primitive.attributes_count > 0);

    cgltf_size vertexCount = primitive.attributes[0].data->count;

    outMesh.vertices.resize(vertexCount);

    int positionAttributeIndex = -1;
    int normalAttributeIndex = -1;
    int textureAttributeIndex = -1;
    int tangentAttributeIndex = -1;
    int boneIDsAttributeIndex = -1;
    int boneWeightsAttributeIndex = -1;
    
    for (cgltf_size i = 0; i < primitive.attributes_count; i++)
    {
        cgltf_attribute_type type = primitive.attributes[i].type;
        switch (type) {
        case cgltf_attribute_type_position:
            positionAttributeIndex = i;
            break;
        case cgltf_attribute_type_normal:
            normalAttributeIndex = i;
            break;
        case cgltf_attribute_type_tangent:
            tangentAttributeIndex = i;
            break;
        case cgltf_attribute_type_texcoord:
            textureAttributeIndex = i;
            break;
        case cgltf_attribute_type_joints:
            boneIDsAttributeIndex = i;
            break;
        case cgltf_attribute_type_weights:
            boneWeightsAttributeIndex = i;
            break;
        case cgltf_attribute_type_invalid:
        case cgltf_attribute_type_color:
        case cgltf_attribute_type_custom:
        case cgltf_attribute_type_max_enum:
            WARN("Attribute {} is unsupported", primitive.attributes[i].name);
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
    if (normalAttributeIndex != -1)
        ASSERT(primitive.attributes[normalAttributeIndex].data->type == cgltf_type_vec3);
    if (textureAttributeIndex != -1)
        ASSERT(primitive.attributes[textureAttributeIndex].data->type == cgltf_type_vec2);
    if (tangentAttributeIndex != -1)
        ASSERT(primitive.attributes[tangentAttributeIndex].data->type == cgltf_type_vec4);
    if (boneIDsAttributeIndex != -1)
        ASSERT(primitive.attributes[boneIDsAttributeIndex].data->type == cgltf_type_vec4);
    if (boneWeightsAttributeIndex != -1)
        ASSERT(primitive.attributes[boneWeightsAttributeIndex].data->type == cgltf_type_vec4);


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
                glm::value_ptr(outMesh.vertices[i].position), 3);

        // Calculate AABB
        for (glm::length_t j = 0; j < outMesh.aabb.min.length(); j++)
        {
            if (outMesh.vertices[i].position[j] > outMesh.aabb.min[j])
                outMesh.aabb.min[j] = outMesh.vertices[i].position[j];
            if (outMesh.vertices[i].position[j] < outMesh.aabb.max[j])
                outMesh.aabb.max[j] = outMesh.vertices[i].position[j];
        }

        if (normalAttributeIndex != -1)
        {
            cgltf_accessor_read_float(primitive.attributes[normalAttributeIndex].data, i,
                    glm::value_ptr(outMesh.vertices[i].normal), 3);
        }

        if (textureAttributeIndex != -1)
        {
            cgltf_accessor_read_float(primitive.attributes[textureAttributeIndex].data, i,
                    glm::value_ptr(outMesh.vertices[i].texture), 2);
        }

        if (tangentAttributeIndex != -1)
        {
            glm::vec4 tangent;
            cgltf_accessor_read_float(primitive.attributes[tangentAttributeIndex].data, i,
                    glm::value_ptr(tangent), 4);
            outMesh.vertices[i].tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
        }

        if (boneIDsAttributeIndex != -1)
        {
            if constexpr (isAnimated)
            {
                outMesh.vertices[i].boneIDs = {0, 0, 0, 0};
                cgltf_accessor_read_uint(primitive.attributes[boneIDsAttributeIndex].data, i,
                        glm::value_ptr(outMesh.vertices[i].boneIDs), 4);
            }
        }

        if (boneWeightsAttributeIndex != -1)
        {
            if constexpr (isAnimated)
            {
                outMesh.vertices[i].boneWeights = {0.0f, 0.0f, 0.0f, 0.0f};
                cgltf_accessor_read_float(primitive.attributes[boneWeightsAttributeIndex].data, i,
                        glm::value_ptr(outMesh.vertices[i].boneWeights), 4);
            }
        }
    }


    outMesh.indices.resize(primitive.indices->count);
    for (cgltf_size i = 0; i < primitive.indices->count; i++)
    {
        outMesh.indices[i] = cgltf_accessor_read_index(primitive.indices, i);
    }
}

inline glm::mat4 convertMatrix(const cgltf_float mat[16])
{
    return {
        mat[0], mat[1], mat[2], mat[3],
        mat[4], mat[5], mat[6], mat[7],
        mat[8], mat[9], mat[10], mat[11],
        mat[12], mat[13], mat[14], mat[15],
    };
}

inline glm::mat4 getTransform(const cgltf_node *node)
{
    glm::mat4 transform;
    if (node->has_matrix)
    {
        transform = convertMatrix(node->matrix);
    }
    else
    {
        glm::quat rotation = glm::identity<glm::quat>();
        if (node->has_rotation)
        {
            rotation = {
                node->rotation[3],
                node->rotation[0],
                node->rotation[1],
                node->rotation[2],
            };
        }

        glm::vec3 translation = {0.0f, 0.0f, 0.0f};
        if (node->has_translation)
        {
            translation = {
                node->translation[0],
                node->translation[1],
                node->translation[2],
            };
        }

        glm::vec3 scale = {1.0f, 1.0f, 1.0f};
        if (node->has_scale)
        {
            scale = {
                node->scale[0],
                node->scale[1],
                node->scale[2],
            };
        }

        transform
            = glm::translate(glm::mat4(1.0f), translation)
            * glm::toMat4(rotation)
            * glm::scale(glm::mat4(1.0f), scale);
    }
    return transform;
}

template<>
std::optional<paca::fileformats::StaticMesh> load<paca::fileformats::StaticMesh>(const char *path)
{
    cgltf_options options {cgltf_file_type_invalid, 0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result != cgltf_result_success)
    {
        ERROR("Error loading GLTF file: {}", path);
        return {};
    }

    result = cgltf_load_buffers(&options, data, path);

    if (result != cgltf_result_success)
    {
        ERROR("Error loading GLTF file buffers for: {}", path);
        return {};
    }

    paca::fileformats::StaticMesh mesh;
    readMesh(data->meshes[0], mesh);

    cgltf_free(data);
    return mesh;
}

void readSkeleton(const cgltf_node &skin, paca::fileformats::Skeleton &outSkeleton, uint32_t parentIndex)
{
    const uint32_t index = outSkeleton.bones.size();
    paca::fileformats::Bone &bone = outSkeleton.bones.emplace_back();
    bone.parentID = parentIndex;
    if (parentIndex == std::numeric_limits<uint32_t>::max())
    {
        bone.offsetMatrix = glm::inverse(getTransform(&skin));
    }
    else
    {
        bone.offsetMatrix = glm::inverse(getTransform(&skin)) * outSkeleton.bones[parentIndex].offsetMatrix;
    }

    ASSERT(skin.name != nullptr);
    outSkeleton.boneNames.push_back(std::string(skin.name));
    for (size_t i = 0; i < skin.children_count; i++)
    {
        cgltf_node* child = skin.children[i];
        readSkeleton(*child, outSkeleton, index);
    }
}

template<>
std::optional<paca::fileformats::AnimatedMesh> load<paca::fileformats::AnimatedMesh>(const char *path)
{
    cgltf_options options {cgltf_file_type_invalid, 0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result != cgltf_result_success)
    {
        ERROR("Error loading GLTF file: {}", path);
        return {};
    }

    result = cgltf_load_buffers(&options, data, path);

    if (result != cgltf_result_success)
    {
        ERROR("Error loading GLTF file buffers for: {}", path);
        return {};
    }

    paca::fileformats::AnimatedMesh mesh;
    readMesh(data->meshes[0], mesh);
    ASSERT(data->skins[0].joints_count > 0);
    readSkeleton(*(data->skins[0].joints[0]), mesh.skeleton, std::numeric_limits<uint32_t>::max());

    cgltf_free(data);
    return mesh;
}

void readAnimation(
    const cgltf_animation &animation,
    paca::fileformats::Animation &outAnimation,
    const std::unordered_map<std::string_view, uint32_t> &boneNameToId)
{
    outAnimation.keyframes.resize(boneNameToId.size());
    float animationDuration = -1.0f;
    size_t numFrames = 0;
    for (cgltf_size s = 0; s < animation.samplers_count; s++)
    {
        cgltf_accessor const* accessor = animation.samplers[s].input;
        ASSERT(accessor->has_max);
        animationDuration = std::max(accessor->max[0], animationDuration);
        numFrames = std::max(accessor->count, numFrames);
    }

    outAnimation.duration = animationDuration;
    outAnimation.ticksPerSecond = (float)numFrames / animationDuration;
    for (size_t channelIndex = 0; channelIndex < animation.channels_count; channelIndex++)
    {
        const cgltf_animation_channel &channel = animation.channels[channelIndex];
        auto it = boneNameToId.find(channel.target_node->name);
        if (it == boneNameToId.end()) continue;

        uint32_t boneIndex = it->second;
        switch (channel.target_path)
        {
            case cgltf_animation_path_type_translation:
            {
                ASSERT(channel.sampler->output->type == cgltf_type_vec3);
                ASSERT(channel.sampler->input->type == cgltf_type_scalar);
                ASSERT(channel.sampler->output->count == channel.sampler->input->count)
                outAnimation.keyframes[boneIndex].positions.resize(
                    channel.sampler->output->count,
                    {
                        .time = 0.0f,
                        .position = {0.0f, 0.0f, 0.0f},
                    });
                for (size_t i = 0; i < channel.sampler->output->count; i++)
                {
                    bool result = cgltf_accessor_read_float(
                        channel.sampler->input,
                        i,
                        &outAnimation.keyframes[boneIndex].positions[i].time,
                        1);
                    ASSERT(result);
                    result = cgltf_accessor_read_float(
                        channel.sampler->output,
                        i,
                        glm::value_ptr(outAnimation.keyframes[boneIndex].positions[i].position),
                        3);
                    ASSERT(result);
                }
                break;
            }
            case cgltf_animation_path_type_rotation:
            {
                ASSERT(channel.sampler->output->type == cgltf_type_vec4);
                ASSERT(channel.sampler->input->type == cgltf_type_scalar);
                ASSERT(channel.sampler->output->count == channel.sampler->input->count)
                outAnimation.keyframes[boneIndex].rotations.resize(
                    channel.sampler->output->count,
                    {
                        .time = 0.0f,
                        .quaternion = glm::identity<glm::quat>(),
                    });
                for (size_t i = 0; i < channel.sampler->output->count; i++)
                {
                    glm::vec4 values;
                    bool result = cgltf_accessor_read_float(
                        channel.sampler->input,
                        i,
                        &outAnimation.keyframes[boneIndex].rotations[i].time,
                        1);
                    ASSERT(result);
                    result = cgltf_accessor_read_float(
                        channel.sampler->output,
                        i,
                        glm::value_ptr(values),
                        4);
                    ASSERT(result);
                    outAnimation.keyframes[boneIndex].rotations[i].quaternion = {
                        values.w,
                        values.x,
                        values.y,
                        values.z,
                    };
                }
                break;
            }
            case cgltf_animation_path_type_scale:
            {
                ASSERT(channel.sampler->output->type == cgltf_type_vec3);
                ASSERT(channel.sampler->input->type == cgltf_type_scalar);
                ASSERT(channel.sampler->output->count == channel.sampler->input->count)
                outAnimation.keyframes[boneIndex].scalings.resize(
                    channel.sampler->output->count,
                    {
                        .time = 0.0f,
                        .scale = {1.0f, 1.0f, 1.0f},
                    });
                for (size_t i = 0; i < channel.sampler->output->count; i++)
                {
                    bool result = cgltf_accessor_read_float(
                        channel.sampler->input,
                        i,
                        &outAnimation.keyframes[boneIndex].scalings[i].time,
                        1);
                    ASSERT(result);
                    result = cgltf_accessor_read_float(
                        channel.sampler->output,
                        i,
                        glm::value_ptr(outAnimation.keyframes[boneIndex].scalings[i].scale),
                        3);
                    ASSERT(result);
                }
                break;
            }
            case cgltf_animation_path_type_invalid:
            case cgltf_animation_path_type_weights:
            case cgltf_animation_path_type_max_enum:
                WARN("Animation path type not supported");
                break;
        }
    }

    //for (auto &keys : outAnimation.keyframes)
    //{
    //    INFO("POSITIONS");
    //    for (size_t i = 0; i < keys.positions.size(); i++)
    //    {
    //        INFO(
    //            "\t{}: ({} {} {}) {}",
    //            i,
    //            keys.positions[i].position.x,
    //            keys.positions[i].position.y,
    //            keys.positions[i].position.z,
    //            keys.positions[i].time);
    //        //keys.positions[i].time = i * secondsPerTick;
    //    }
    //    INFO("ROTATIONS");
    //    for (size_t i = 0; i < keys.rotations.size(); i++)
    //    {
    //        INFO(
    //            "\t{}: ({} {} {}) {}",
    //            i,
    //            keys.rotations[i].quaternion.x,
    //            keys.rotations[i].quaternion.y,
    //            keys.rotations[i].quaternion.z,
    //            keys.rotations[i].quaternion.w,
    //            keys.rotations[i].time);
    //        //keys.rotations[i].time = i * secondsPerTick;
    //    }
    //    INFO("SCALES");
    //    for (size_t i = 0; i < keys.scalings.size(); i++)
    //    {
    //        INFO(
    //            "\t{}: ({} {} {}) {}",
    //            i,
    //            keys.positions[i].position.x,
    //            keys.positions[i].position.y,
    //            keys.positions[i].position.z,
    //            keys.positions[i].time);
    //        //keys.scalings[i].time = i * secondsPerTick;
    //    }
    //}
}

template<>
std::optional<paca::fileformats::Animation> load<paca::fileformats::Animation>(const char *path)
{
    cgltf_options options {cgltf_file_type_invalid, 0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);

    if (result != cgltf_result_success)
    {
        ERROR("Error loading GLTF file: {}", path);
        return {};
    }

    result = cgltf_load_buffers(&options, data, path);

    if (result != cgltf_result_success)
    {
        ERROR("Error loading GLTF file buffers for: {}", path);
        return {};
    }

    paca::fileformats::Skeleton skeleton;
    ASSERT(data->skins[0].joints_count > 0);
    readSkeleton(*(data->skins[0].joints[0]), skeleton, std::numeric_limits<uint32_t>::max());
    std::unordered_map<std::string_view, uint32_t> boneNameToIndex;
    for (uint32_t i = 0; i < skeleton.boneNames.size(); i++)
    {
        boneNameToIndex.emplace(skeleton.boneNames[i].c_str(), i);
    }

    paca::fileformats::Animation animation;
    ASSERT(data->animations_count > 0);
    readAnimation(data->animations[0], animation, boneNameToIndex);

    cgltf_free(data);
    return animation;
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
