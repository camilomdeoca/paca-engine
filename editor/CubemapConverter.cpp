#include "CubemapConverter.hpp"
#include "utils/Log.hpp"

#include <cstring>
#include <filesystem>
#include <stb_image.h>

paca::fileformats::AssetPack cubemapToPacaFormat(const std::string &folder, const std::string &outName)
{
    paca::fileformats::AssetPack pack;

    const std::array<std::string, 6> facesNames = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg"
    };

    pack.cubeMaps.emplace_back();
    paca::fileformats::CubeMap &cubeMap = pack.cubeMaps[0];
    std::array<std::string, 6> filePaths;
    filePaths[0] = std::filesystem::path(folder) / facesNames[0];
    int width, height, channels;

    int result = stbi_info(filePaths[0].c_str(), &width, &height, &channels);
    if (!result)
        ERROR("Couldnt load image: {}", filePaths[0]);

    for (unsigned int i = 1; i < facesNames.size(); i++)
    {
        filePaths[i] = std::filesystem::path(folder) / facesNames[i];
        int newWidth, newHeight, newChannels;
        int result = stbi_info(filePaths[i].c_str(), &newWidth, &newHeight, &newChannels);

        if (!result)
            ERROR("Couldnt load image: {}", filePaths[0]);

        if (newWidth != width || newHeight != height || newChannels != channels)
            ERROR("Cubemap image: {}, has different size", filePaths[i]);
    }

    cubeMap.pixelData.resize(width*height*channels*6);

    for (unsigned int i = 0; i < facesNames.size(); i++)
    {
        stbi_set_flip_vertically_on_load(0);

        stbi_uc *data = stbi_load(filePaths[i].c_str(), &width, &height, &channels, 0);

        if (!data)
            ERROR("Failed to load image: {}!", filePaths[i].c_str()); 

        INFO("{} ({}x{}) has {} channels.", filePaths[i].c_str(), width, height, channels);

        memcpy(
            cubeMap.pixelData.data() + width * height * channels * i,
            data,
            width * height * channels);

        stbi_image_free(data);
    }

    cubeMap.width = width;
    cubeMap.height = height;
    cubeMap.channels = channels;
    cubeMap.name = outName;
    cubeMap.id = 0;

    return pack;
}
