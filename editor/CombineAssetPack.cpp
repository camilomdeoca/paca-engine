#include "CombineAssetPack.hpp"

#include <set>

paca::fileformats::AssetPack combine(
    std::vector<std::reference_wrapper<paca::fileformats::AssetPack>> assetPacks)
{
    paca::fileformats::AssetPack result;

    std::set<paca::fileformats::StaticMeshId> staticMeshIds;
    std::set<paca::fileformats::AnimatedMeshId> animatedMeshIds;
    std::set<paca::fileformats::MaterialId> materialIds;
    std::set<paca::fileformats::TextureId> textureIds;
    std::set<paca::fileformats::CubeMapId> cubeMapIds;
    std::set<paca::fileformats::AnimationId> animationIds;
    std::set<paca::fileformats::FontId> fontIds;

    for (paca::fileformats::AssetPack &resourcePack : assetPacks)
    {

        paca::fileformats::StaticMeshId staticMeshIdOffset;
        if (staticMeshIds.empty()) staticMeshIdOffset = 0;
        else staticMeshIdOffset = *staticMeshIds.rbegin() + 1;

        paca::fileformats::AnimatedMeshId animatedMeshIdOffset;
        if (animatedMeshIds.empty()) animatedMeshIdOffset = 0;
        else animatedMeshIdOffset = *animatedMeshIds.rbegin() + 1;

        paca::fileformats::MaterialId materialIdOffset;
        if (materialIds.empty()) materialIdOffset = 0;
        else materialIdOffset = *materialIds.rbegin() + 1;

        paca::fileformats::TextureId textureIdOffset;
        if (textureIds.empty()) textureIdOffset = 0;
        else textureIdOffset = *textureIds.rbegin() + 1;

        paca::fileformats::CubeMapId cubeMapIdOffset;
        if (cubeMapIds.empty()) cubeMapIdOffset = 0;
        else cubeMapIdOffset = *textureIds.rbegin() + 1;

        paca::fileformats::AnimationId animationIdOffset;
        if (animationIds.empty()) animationIdOffset = 0;
        else animationIdOffset = *animationIds.rbegin() + 1;

        paca::fileformats::FontId fontIdOffset;
        if (fontIds.empty()) fontIdOffset = 0;
        else fontIdOffset = *fontIds.rbegin() + 1;

        for (paca::fileformats::StaticMesh &staticMesh : resourcePack.staticMeshes)
        {
            staticMesh.id += staticMeshIdOffset;
            staticMesh.materialId += materialIdOffset;
            staticMeshIds.emplace(staticMesh.id);
            result.staticMeshes.push_back(staticMesh);
        }

        for (paca::fileformats::AnimatedMesh &animatedMesh : resourcePack.animatedMeshes)
        {
            animatedMesh.id += animatedMeshIdOffset;
            animatedMesh.materialId += materialIdOffset;
            for (paca::fileformats::AnimationId &animationId : animatedMesh.animations)
                animationId += animationIdOffset;
            animatedMeshIds.emplace(animatedMesh.id);
            result.animatedMeshes.push_back(animatedMesh);
        }

        for (paca::fileformats::Material &material : resourcePack.materials)
        {
            material.id += materialIdOffset;
            for (std::vector<paca::fileformats::TextureId> &texturesIdsOfType : material.textures)
                for (paca::fileformats::TextureId &textureId : texturesIdsOfType)
                    textureId += textureIdOffset;
            materialIds.emplace(material.id);
            result.materials.push_back(material);
        }

        for (paca::fileformats::Texture &texture : resourcePack.textures)
        {
            texture.id += textureIdOffset;
            textureIds.emplace(texture.id);
            result.textures.push_back(texture);
        }

        for (paca::fileformats::CubeMap &cubeMap : resourcePack.cubeMaps)
        {
            cubeMap.id += cubeMapIdOffset;
            cubeMapIds.emplace(cubeMap.id);
            result.cubeMaps.push_back(cubeMap);
        }
        
        for (paca::fileformats::Animation &animation : resourcePack.animations)
        {
            animation.id += animationIdOffset;
            animationIds.emplace(animation.id);
            result.animations.push_back(animation);
        }

        for (paca::fileformats::Font &font : resourcePack.fonts)
        {
            font.id += fontIdOffset;
            fontIds.emplace(font.id);
            result.fonts.push_back(font);
        }
    }
    return result;
}
