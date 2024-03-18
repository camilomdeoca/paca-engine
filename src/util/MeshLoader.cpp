#include "MeshLoader.hpp"

#include <cstddef>
#include <fstream>
#include <string>

std::optional<MeshData> MeshLoader::loadObj(const std::string &path)
{
    std::ifstream file(path);
    if(!file)
        return {};

    MeshData result;
    std::vector<float> vertices, textureCoords, normals;
    std::vector<uint32_t> vertexIndices, textureIndices, normalIndices;

    while (!file.eof())
    {
        std::string currentLine;
        std::getline(file, currentLine);

        switch (currentLine[0]) {
        case 'v':
        {
            switch (currentLine[1]) {
            case ' ':
            {
                size_t charsProcessedTotal = 2;
                size_t charsProcessed;
                for (unsigned int i = 0; i < 3; i++)
                {
                    vertices.push_back(std::stof(currentLine.substr(charsProcessedTotal), &charsProcessed));
                    charsProcessedTotal += charsProcessed;
                }
                break;
            }
            case 't':
            {
                size_t charsProcessedTotal = 3;
                size_t charsProcessed;
                for (unsigned int i = 0; i < 2; i++)
                {
                    textureCoords.push_back(std::stof(currentLine.substr(charsProcessedTotal), &charsProcessed));
                    charsProcessedTotal += charsProcessed;
                }
                break;
            }
            case 'n':
            {
                size_t charsProcessedTotal = 3;
                size_t charsProcessed;
                for (unsigned int i = 0; i < 3; i++)
                {
                    normals.push_back(std::stof(currentLine.substr(charsProcessedTotal), &charsProcessed));
                    charsProcessedTotal += charsProcessed;
                }
                break;
            }
            default: break;
            }
            break;
        }
        case 'f':
        {
            if (!textureCoords.empty() && !normals.empty()) // the indices have format: f v/vt/vn
            {
                size_t charsProcessedTotal = 2;
                size_t charsProcessed;
                for (unsigned int i = 0; i < 3; i++)
                {
                    vertexIndices.push_back(std::stoul(currentLine.substr(charsProcessedTotal), &charsProcessed));
                    charsProcessedTotal += charsProcessed;
                    textureIndices.push_back(std::stoul(currentLine.substr(charsProcessedTotal+1), &charsProcessed));
                    charsProcessedTotal += charsProcessed + 1;
                    normalIndices.push_back(std::stoul(currentLine.substr(charsProcessedTotal+1), &charsProcessed));
                    charsProcessedTotal += charsProcessed + 1;
                }
            }
            else if (!textureCoords.empty() && normals.empty()) // format: f v/vt
            {
                size_t charsProcessedTotal = 2;
                size_t charsProcessed;
                for (unsigned int i = 0; i < 3; i++)
                {
                    vertexIndices.push_back(std::stoul(currentLine.substr(charsProcessedTotal), &charsProcessed));
                    charsProcessedTotal += charsProcessed;
                    textureIndices.push_back(std::stoul(currentLine.substr(charsProcessedTotal+1), &charsProcessed));
                    charsProcessedTotal += charsProcessed + 1;
                }
            }
            else if (textureCoords.empty() && !normals.empty()) // format: f v//vn
            {
                size_t charsProcessedTotal = 2;
                size_t charsProcessed;
                for (unsigned int i = 0; i < 3; i++)
                {
                    vertexIndices.push_back(std::stoul(currentLine.substr(charsProcessedTotal), &charsProcessed));
                    charsProcessedTotal += charsProcessed;
                    normalIndices.push_back(std::stoul(currentLine.substr(charsProcessedTotal+2), &charsProcessed));
                    charsProcessedTotal += charsProcessed + 2;
                }
            }
            else if (textureCoords.empty() && normals.empty()) // format: f v
            {
                size_t charsProcessedTotal = 2;
                size_t charsProcessed;
                for (unsigned int i = 0; i < 3; i++)
                {
                    vertexIndices.push_back(std::stoul(currentLine.substr(charsProcessedTotal), &charsProcessed));
                    charsProcessedTotal += charsProcessed;
                }
                break;
            }
            else
            {
                fprintf(stderr, "Bad obj file: vertices have to exist before specifying indices.\n");
                return {};
            }
        }
        default: break;
        }
    }
    if (!textureCoords.empty() && !normals.empty()) // There is textureCoords and normals
    {
        if (vertexIndices.size() != textureIndices.size() || vertexIndices.size() != normalIndices.size())
        {
            fprintf(stderr, "vertex, texture coordinates and normal indices list of obj file should have the same size.\n");
            exit(1);
        }
        uint32_t index = 0;
        for (size_t i = 0; i < vertexIndices.size(); i++) {
            result.vertices.push_back(vertices[(vertexIndices[i] - 1) * 3 + 0]);
            result.vertices.push_back(vertices[(vertexIndices[i] - 1) * 3 + 1]);
            result.vertices.push_back(vertices[(vertexIndices[i] - 1) * 3 + 2]);
            result.vertices.push_back(normals[(normalIndices[i] - 1) * 3 + 0]);
            result.vertices.push_back(normals[(normalIndices[i] - 1) * 3 + 1]);
            result.vertices.push_back(normals[(normalIndices[i] - 1) * 3 + 2]);
            result.vertices.push_back(textureCoords[(textureIndices[i] - 1) * 2 + 0]);
            result.vertices.push_back(textureCoords[(textureIndices[i] - 1) * 2 + 1]);
            result.indices.push_back(index++);
        }
    }
    else if (textureCoords.empty() && !normals.empty()) // There is normals but not textureCoords
    {
        if (vertexIndices.size() != normalIndices.size())
        {
            fprintf(stderr, "vertex and normal indices list of obj file should have the same size.\n");
            exit(1);
        }
        uint32_t index = 0;
        for (size_t i = 0; i < vertexIndices.size(); i++) {
            result.vertices.push_back(vertices[(vertexIndices[i] - 1) * 3 + 0]);
            result.vertices.push_back(vertices[(vertexIndices[i] - 1) * 3 + 1]);
            result.vertices.push_back(vertices[(vertexIndices[i] - 1) * 3 + 2]);
            result.vertices.push_back(normals[(normalIndices[i] - 1) * 3 + 0]);
            result.vertices.push_back(normals[(normalIndices[i] - 1) * 3 + 1]);
            result.vertices.push_back(normals[(normalIndices[i] - 1) * 3 + 2]);
            result.vertices.push_back(0);
            result.vertices.push_back(0);
            result.indices.push_back(index++);
        }
    }

    return result;
}
