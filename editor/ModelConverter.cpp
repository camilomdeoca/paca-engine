#include "ModelConverter.hpp"

#include <serializers/Concepts.hpp>
#include <utils/Log.hpp>
#include <utils/Assert.hpp>

#include <boost/pfr/core.hpp>

#include <assimp/Importer.hpp>
#include <assimp/anim.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <stb_image.h>

#include <type_traits>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <filesystem>

class FileConverter
{
public:
    FileConverter(const std::string &path);

    paca::fileformats::AssetPack &&getAssetPack() { return std::move(m_pack); }

    size_t getSize() const { return m_size;}

private:
    struct BoneData {
        uint32_t id;
        glm::mat4 offset;
    };

    std::vector<std::string> loadMaterialTextures(
        aiMaterial *mat,
        aiTextureType type);

    void processMaterial(
        aiMaterial *material,
        unsigned int indexOfMaterial);

    paca::fileformats::Animation processAnimation(
        aiAnimation *assimpAnim,
        unsigned int indexOfAnimation);

    void processBoneChilds(
        aiNode *node,
        paca::fileformats::Skeleton &outSkeleton,
        const std::unordered_map<std::string, BoneData> &bonesData,
        uint32_t parentID);

    paca::fileformats::Mesh processMesh(
        aiMesh *mesh,
        const aiScene *scene);

    void processNode(
        aiNode *node,
        const aiScene *scene);

    paca::fileformats::AssetPack m_pack;
    std::unordered_set<unsigned int> m_usedMaterialIndexes;
    std::string m_path;
    std::string m_outName;
    size_t m_size;

};

static inline glm::mat4 toGlmMatrix(const aiMatrix4x4& from)
{
    glm::mat4 to;
    //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

static inline glm::vec3 toGlmVec(const aiVector3D& vec)
{
    return glm::vec3(vec.x, vec.y, vec.z);
}


static inline glm::quat toGlmQuat(const aiQuaternion& quat)
{
    return glm::quat(quat.w, quat.x, quat.y, quat.z);
}

/*!
 * @param basePath The path where the textures will be searched at.
 */
std::vector<std::string> FileConverter::loadMaterialTextures(
    aiMaterial *mat,
    aiTextureType type)
{
    auto addInFileSize = [this]<typename T>(T &field) {
        if constexpr (paca::fileformats::dynamic_array<T>)
        {
            m_size += sizeof(field.size());
        }
        else if constexpr (std::is_arithmetic_v<T> || paca::fileformats::glm_type<T>)
        {
            m_size += sizeof(field);
        }
    };

    std::filesystem::path basePath = std::filesystem::path(m_path).parent_path();
    std::vector<std::string> textureNames;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string path = str.C_Str();
        textureNames.push_back(path);
        m_size += sizeof(textureNames.back().size());
        m_size += sizeof(textureNames.back()[0]) * textureNames.back().size();
        {
            paca::fileformats::Texture texture;
            boost::pfr::for_each_field(texture, addInFileSize);
            std::filesystem::path fullPath = basePath / std::filesystem::path(path);
            int width, height, channels;
            stbi_set_flip_vertically_on_load(1);
            stbi_uc *data = stbi_load(fullPath.c_str(), &width, &height, &channels, 0);

            ASSERT_MSG(data, "Failed to load image: {}!", fullPath.c_str()); 
            INFO("{} ({}x{}) has {} channels.", fullPath.c_str(), width, height, channels);

            texture.name = path;
            texture.width = width;
            texture.height = height;
            texture.channels = channels;
            texture.isCubeMap = false;
            texture.pixelData.resize(width * height * channels);
            std::memcpy(texture.pixelData.data(), data, width * height * channels);

            stbi_image_free(data);

            m_size += sizeof(texture.name[0]) * texture.name.size();
            m_size += sizeof(texture.pixelData[0]) * texture.pixelData.size();
            m_pack.textures.emplace_back(std::move(texture));
        }
    }
    return textureNames;
}

void FileConverter::processMaterial(
    aiMaterial *material,
    unsigned int indexOfMaterial)
{
    auto addInFileSize = [this]<typename T>(T &field) {
        if constexpr (paca::fileformats::dynamic_array<T>)
        {
            m_size += sizeof(field.size());
        }
        else if constexpr (std::is_arithmetic_v<T> || paca::fileformats::glm_type<T>)
        {
            m_size += sizeof(field);
        }
    };

    paca::fileformats::Material result;
    boost::pfr::for_each_field(result, addInFileSize);
    result.textures[paca::fileformats::TextureType::diffuse] = loadMaterialTextures(material, aiTextureType_DIFFUSE);
    result.textures[paca::fileformats::TextureType::specular] = loadMaterialTextures(material, aiTextureType_SPECULAR);
    result.textures[paca::fileformats::TextureType::normal] = loadMaterialTextures(material, aiTextureType_NORMALS);
    result.textures[paca::fileformats::TextureType::depth] = loadMaterialTextures(material, aiTextureType_HEIGHT);
    m_size += sizeof(result.textures[0].size()) * result.textures.size();

    result.name = m_outName + "Material" + std::to_string(indexOfMaterial);
    m_size += sizeof(result.name[0]) * result.name.size();

    m_pack.materials.push_back(result);
}

paca::fileformats::Animation FileConverter::processAnimation(
        aiAnimation *assimpAnim,
        unsigned int indexOfAnimation)
{
    auto addInFileSize = [this]<typename T>(T &field) {
        if constexpr (paca::fileformats::dynamic_array<T>)
        {
            m_size += sizeof(field.size());
        }
        else if constexpr (std::is_arithmetic_v<T> || paca::fileformats::glm_type<T>)
        {
            m_size += sizeof(field);
        }
    };

    std::unordered_map<std::string, uint32_t> boneNameToId;
    for (unsigned int boneId = 0; boneId < m_pack.models[0].meshes[0].skeleton.boneNames.size(); boneId++)
    {
        boneNameToId.emplace(std::make_pair(m_pack.models[0].meshes[0].skeleton.boneNames[boneId], boneId));
    }

    paca::fileformats::Animation outAnimation;
    boost::pfr::for_each_field(outAnimation, addInFileSize);
    outAnimation.name = std::string(assimpAnim->mName.C_Str()) + "Animation" + std::to_string(indexOfAnimation);
    m_size += sizeof(outAnimation.name[0]) * outAnimation.name.size();
    outAnimation.duration = assimpAnim->mDuration;
    outAnimation.ticksPerSecond = assimpAnim->mTicksPerSecond;
    outAnimation.keyframes.resize(boneNameToId.size());
    // For each bone (There is a channel with keyframes for each bone)
    for (unsigned int i = 0; i < assimpAnim->mNumChannels; i++)
    {
        std::unordered_map<std::string, uint32_t>::const_iterator it = boneNameToId.find(assimpAnim->mChannels[i]->mNodeName.C_Str());
        if (it == boneNameToId.end())
        {
            printf("Error: animation channel for non bone node.\n");
            continue;
        }
        uint32_t boneId = it->second;
        // For each position keyframe
        for (unsigned int keyFrameIndex = 0; keyFrameIndex < assimpAnim->mChannels[i]->mNumPositionKeys; keyFrameIndex++)
        {
            outAnimation.keyframes[boneId].positions.emplace_back(
                    assimpAnim->mChannels[i]->mPositionKeys[keyFrameIndex].mTime,
                    toGlmVec(assimpAnim->mChannels[i]->mPositionKeys[keyFrameIndex].mValue));
        }
        // For each rotation keyframe
        for (unsigned int keyFrameIndex = 0; keyFrameIndex < assimpAnim->mChannels[i]->mNumRotationKeys; keyFrameIndex++)
        {
            outAnimation.keyframes[boneId].rotations.emplace_back(
                    assimpAnim->mChannels[i]->mRotationKeys[keyFrameIndex].mTime,
                    toGlmQuat(assimpAnim->mChannels[i]->mRotationKeys[keyFrameIndex].mValue));
        }
        // For each scaling keyframe
        for (unsigned int keyFrameIndex = 0; keyFrameIndex < assimpAnim->mChannels[i]->mNumScalingKeys; keyFrameIndex++)
        {
            outAnimation.keyframes[boneId].scalings.emplace_back(
                    assimpAnim->mChannels[i]->mScalingKeys[keyFrameIndex].mTime,
                    toGlmVec(assimpAnim->mChannels[i]->mScalingKeys[keyFrameIndex].mValue));
        }

        m_size += sizeof(outAnimation.keyframes[boneId].positions.size());
        m_size += sizeof(outAnimation.keyframes[boneId].positions[0])
            * outAnimation.keyframes[boneId].positions.size();
        m_size += sizeof(outAnimation.keyframes[boneId].rotations.size());
        m_size += sizeof(outAnimation.keyframes[boneId].rotations[0])
            * outAnimation.keyframes[boneId].rotations.size();
        m_size += sizeof(outAnimation.keyframes[boneId].scalings.size());
        m_size += sizeof(outAnimation.keyframes[boneId].scalings[0])
            * outAnimation.keyframes[boneId].scalings.size();
    }
    return outAnimation;
}

struct BoneData {
    uint32_t id;
    glm::mat4 offset;
};

void FileConverter::processBoneChilds(
        aiNode *node,
        paca::fileformats::Skeleton &outSkeleton,
        const std::unordered_map<std::string, BoneData> &bonesData,
        uint32_t parentID)
{
    auto it = bonesData.find(node->mName.C_Str());
    if (it != bonesData.end()) // if node is a bone
    {
        outSkeleton.bones.emplace_back(parentID, it->second.offset);
        outSkeleton.boneNames.emplace_back(node->mName.C_Str());
        m_size += sizeof(outSkeleton.bones.back());
        m_size += sizeof(outSkeleton.boneNames.back().size());
        m_size += sizeof(outSkeleton.boneNames.back()[0]) * outSkeleton.boneNames.back().size();
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
			processBoneChilds(node->mChildren[i], outSkeleton, bonesData, it->second.id);
		}
    }
    else
    {
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
			processBoneChilds(node->mChildren[i], outSkeleton, bonesData, parentID);
		}
    }
}

template<typename T>
void insertIntoCharVector(T value, std::vector<uint8_t> &vector)
{
    const uint8_t *bytes = reinterpret_cast<const uint8_t*>(&value);
    vector.insert(vector.end(), bytes, bytes + sizeof(T));
}

template<typename T>
void insertIntoCharVector(T value, std::vector<uint8_t> &vector, std::vector<uint8_t>::iterator position)
{
    const uint8_t *bytes = reinterpret_cast<const uint8_t*>(&value);
    vector.insert(position, bytes, bytes + sizeof(T));
}

paca::fileformats::Mesh FileConverter::processMesh(
        aiMesh *mesh,
        const aiScene *scene)
{
    constexpr size_t MAX_BONES_INFLUENCE = 4;

    auto addInFileSize = [this]<typename T>(T &field) {
        if constexpr (paca::fileformats::dynamic_array<T>)
        {
            m_size += sizeof(field.size());
        }
        else if constexpr (std::is_arithmetic_v<T> || paca::fileformats::glm_type<T>)
        {
            m_size += sizeof(field);
        }
    };

    paca::fileformats::Mesh result;
    boost::pfr::for_each_field(result, addInFileSize);

    if (mesh->HasBones())
        result.vertexType = paca::fileformats::VertexType::float3pos_float3norm_float3tang_float2texture_int4boneIds_float4boneWeights;
    else if (mesh->HasTangentsAndBitangents())
        result.vertexType = paca::fileformats::VertexType::float3pos_float3norm_float3tang_float2texture;
    else
        result.vertexType = paca::fileformats::VertexType::float3pos_float3norm_float2texture;

    struct BoneInfluence {
        int32_t boneID;
        float weight;
    };

    // Get bones influences
    std::vector<std::vector<BoneInfluence>> bonesInfluences(mesh->mNumVertices); // The index is vertexID
    for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
    {
        for (unsigned int weightIndex = 0; weightIndex < mesh->mBones[boneIndex]->mNumWeights; weightIndex++)
        {
            unsigned int vertexID = mesh->mBones[boneIndex]->mWeights[weightIndex].mVertexId;
            float weight = mesh->mBones[boneIndex]->mWeights[weightIndex].mWeight;
            bonesInfluences[vertexID].emplace_back(mesh->mNumBones - 1 - boneIndex, weight);

            //Bone &newBone = result.bones.emplace_back();
            //std::memcpy(newBone.offset, &mesh->mBones[boneIndex]->mOffsetMatrix, sizeof(newBone.offset));
            //newBone.id = boneIndex;
            //newBone.name = mesh->mBones[boneIndex]->mName.C_Str();
        }
    }

    // Get vertices
    for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++)
    {
        insertIntoCharVector(mesh->mVertices[vertexIndex].x, result.vertices);
        insertIntoCharVector(mesh->mVertices[vertexIndex].y, result.vertices);
        insertIntoCharVector(mesh->mVertices[vertexIndex].z, result.vertices);

        bool hasNormals = mesh->HasNormals();
        insertIntoCharVector(hasNormals ? mesh->mNormals[vertexIndex].x : 0, result.vertices);
        insertIntoCharVector(hasNormals ? mesh->mNormals[vertexIndex].y : 0, result.vertices);
        insertIntoCharVector(hasNormals ? mesh->mNormals[vertexIndex].z : 0, result.vertices);

        bool hasTangents = mesh->HasTangentsAndBitangents();
        insertIntoCharVector(hasTangents ? mesh->mTangents[vertexIndex].x : 0, result.vertices);
        insertIntoCharVector(hasTangents ? mesh->mTangents[vertexIndex].y : 0, result.vertices);
        insertIntoCharVector(hasTangents ? mesh->mTangents[vertexIndex].z : 0, result.vertices);

        bool hasTextureCoords = mesh->mTextureCoords[0];
        insertIntoCharVector(hasTextureCoords ? mesh->mTextureCoords[0][vertexIndex].x : 0, result.vertices);
        insertIntoCharVector(hasTextureCoords ? mesh->mTextureCoords[0][vertexIndex].y : 0, result.vertices);

        if (mesh->HasBones())
        {
            // Set bone IDs
            for (size_t i = 0; i < MAX_BONES_INFLUENCE; i++)
            {
                if (i < bonesInfluences[vertexIndex].size())
                    insertIntoCharVector(bonesInfluences[vertexIndex][i].boneID, result.vertices);
                else
                    insertIntoCharVector(static_cast<uint32_t>(std::numeric_limits<uint32_t>::max()), result.vertices);
                //assert(bonesInfluences[vertexIndex].size() <= MAX_BONES_INFLUENCE && "Too much bones influence a vertex");
            }
            // Set bone weights
            for (size_t i = 0; i < MAX_BONES_INFLUENCE; i++)
            {
                if (i < bonesInfluences[vertexIndex].size())
                    insertIntoCharVector(bonesInfluences[vertexIndex][i].weight, result.vertices);
                else
                    insertIntoCharVector(static_cast<float>(0.0f), result.vertices);
                //assert(bonesInfluences[vertexIndex].size() <= MAX_BONES_INFLUENCE && "Too much bones influence a vertex");
            }
        }

        m_size += paca::fileformats::vertexTypeToSize(result.vertexType);
    }

    // Get indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            result.indices.push_back(face.mIndices[j]);
            m_size += sizeof(result.indices.back());
        }
    }

    if (mesh->mNumBones > 0)
    {
        std::unordered_map<std::string, BoneData> bonesData;
        for (unsigned int boneId = 0; boneId < mesh->mNumBones; boneId++)
        {
            bonesData.emplace(std::make_pair(
                        mesh->mBones[boneId]->mName.C_Str(),
                        BoneData{
                            mesh->mNumBones - 1 - boneId,
                            toGlmMatrix(mesh->mBones[boneId]->mOffsetMatrix)
                        }));
        }
        processBoneChilds(scene->mRootNode, result.skeleton, bonesData, std::numeric_limits<uint32_t>::max());
    }
    boost::pfr::for_each_field(result.skeleton, addInFileSize);

    if (scene->HasAnimations())
    {
        for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; animIndex++)
        {
            result.animations.emplace_back(std::string(scene->mAnimations[animIndex]->mName.C_Str()) + "Animation" + std::to_string(animIndex));
            m_size += sizeof(result.animations.back().size());
            m_size += sizeof(result.animations.back()[0]) * result.animations.back().size();
        }
    }

    m_usedMaterialIndexes.insert(mesh->mMaterialIndex);

    result.materialName = m_outName + "Material" + std::to_string(mesh->mMaterialIndex);
    m_size += sizeof(result.materialName[0]) * result.materialName.size();
    result.indexType = paca::fileformats::IndexType::uint32bit;

    return result;
}

void FileConverter::processNode(
        aiNode *node,
        const aiScene *scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        m_pack.models[0].meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

FileConverter::FileConverter(const std::string &path)
    : m_path(path), m_outName(path), m_size(0)
{
    auto addInFileSize = [this]<typename T>(T &field) {
        if constexpr (paca::fileformats::dynamic_array<T>)
        {
            m_size += sizeof(field.size());
        }
        else if constexpr (std::is_arithmetic_v<T> || paca::fileformats::glm_type<T>)
        {
            m_size += sizeof(field);
        }
    };

    boost::pfr::for_each_field(m_pack, addInFileSize);
    
    m_pack.models.emplace_back();
    boost::pfr::for_each_field(m_pack.models.back(), addInFileSize);

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        ERROR("{}", importer.GetErrorString());
        ERROR("Error loading file: {}.", path.c_str());
    }

    processNode(scene->mRootNode, scene);

    if (scene->HasAnimations())
    {
        for (unsigned int animIndex = 0; animIndex < scene->mNumAnimations; animIndex++)
        {
            m_pack.animations.emplace_back(processAnimation(scene->mAnimations[animIndex], animIndex));
        }
    }

    for (const unsigned int &i : m_usedMaterialIndexes)
        processMaterial(scene->mMaterials[i], i);


    m_pack.models[0].name = m_outName;
    m_size += sizeof(m_pack.models[0].name[0]) * m_pack.models[0].name.size();
}

paca::fileformats::AssetPack modelToPacaFormat(const std::string &path)
{
    FileConverter fileConverter(path);
    return fileConverter.getAssetPack();
}

paca::fileformats::AssetPack modelToPacaFormat(const std::string &path, size_t &size)
{
    FileConverter fileConverter(path);
    size = fileConverter.getSize();
    return fileConverter.getAssetPack();
}

