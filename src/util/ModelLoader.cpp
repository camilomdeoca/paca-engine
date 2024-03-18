#include "ModelLoader.hpp"
#include "engine/Material.hpp"
#include "engine/ResourceManager.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <cstdio>
#include <memory>
#include <optional>
#include <vector>

std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &directory)
{
    std::vector<std::shared_ptr<Texture>> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::shared_ptr<Texture> texture = ResourceManager::getTexture(directory + '/' + str.C_Str());
        textures.push_back(texture);
    }
    return textures;
}

std::shared_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene, const std::string &directory)
{
    std::vector<float> vertices;
    std::vector<uint32_t> indices;

    // Get vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        bool hasNormals = mesh->HasNormals();
        vertices.push_back(hasNormals ? mesh->mNormals[i].x : 0);
        vertices.push_back(hasNormals ? mesh->mNormals[i].y : 0);
        vertices.push_back(hasNormals ? mesh->mNormals[i].z : 0);

        bool hasTextureCoords = mesh->mTextureCoords[0];
        vertices.push_back(hasTextureCoords ? mesh->mTextureCoords[0][i].x : 0);
        vertices.push_back(hasTextureCoords ? mesh->mTextureCoords[0][i].y : 0);
    }

    // Get indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    MaterialSpecification materialSpec;
    materialSpec.textureMaps[MaterialTextureType::diffuse] = loadMaterialTextures(material, aiTextureType_DIFFUSE, directory);
    materialSpec.textureMaps[MaterialTextureType::specular] = loadMaterialTextures(material, aiTextureType_SPECULAR, directory);

    return std::make_shared<Mesh>(vertices, indices, std::make_shared<Material>(materialSpec));
}

void processNode(aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<Mesh>> &meshesOut, const std::string &directory)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshesOut.push_back(processMesh(mesh, scene, directory));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, meshesOut, directory);
    }
}

std::optional<ModelData> ModelLoader::loadModel(const std::string &path)
{
    ModelData result;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fprintf(stderr, "%s", importer.GetErrorString());
        fprintf(stderr, "Error loading file: %s.\n", path.c_str());
        return {};
    }

    std::string directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene, result.meshes, directory);

    return result;
}

