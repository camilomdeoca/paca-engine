#include "AssetMetadataManager.hpp"
#include "engine/IdTypes.hpp"

#include <engine/Loader.hpp>
#include <utils/Assert.hpp>

void AssetMetadataManager::init()
{
    m_previewRenderer.init();
}

const StaticMeshMetadata *AssetMetadataManager::get(StaticMeshId id) const
{
    const auto it = m_staticMeshes.find(id);
    if (it != m_staticMeshes.end())
        return &it->second;
    return nullptr;
}

const AnimatedMeshMetadata *AssetMetadataManager::get(AnimatedMeshId id) const
{
    const auto it = m_animatedMeshes.find(id);
    if (it != m_animatedMeshes.end())
        return &it->second;
    return nullptr;
}

const AnimationMetadata *AssetMetadataManager::get(AnimationId id) const
{
    const auto it = m_animations.find(id);
    if (it != m_animations.end())
        return &it->second;
    return nullptr;
}

const MaterialMetadata *AssetMetadataManager::get(MaterialId id) const
{
    const auto it = m_materials.find(id);
    if (it != m_materials.end())
        return &it->second;
    return nullptr;
}

bool AssetMetadataManager::remove(StaticMeshId id)
{
    m_assetManager.remove(id);
    return m_staticMeshes.erase(id);
}

bool AssetMetadataManager::remove(AnimatedMeshId id)
{
    m_assetManager.remove(id);
    return m_animatedMeshes.erase(id);
}

bool AssetMetadataManager::remove(AnimationId id)
{
    m_assetManager.remove(id);
    return m_animations.erase(id);
}

bool AssetMetadataManager::remove(MaterialId id)
{
    m_assetManager.remove(id);
    return m_materials.erase(id);
}

bool AssetMetadataManager::move(StaticMeshId from, StaticMeshId to)
{
    m_assetManager.move(from, to);

    if (m_staticMeshes.contains(to)) return false;
    auto node = m_staticMeshes.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_staticMeshes.insert(std::move(node)).inserted;
}

bool AssetMetadataManager::move(AnimatedMeshId from, AnimatedMeshId to)
{
    m_assetManager.move(from, to);

    if (m_animatedMeshes.contains(to)) return false;
    auto node = m_animatedMeshes.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_animatedMeshes.insert(std::move(node)).inserted;
}

bool AssetMetadataManager::move(AnimationId from, AnimationId to)
{
    m_assetManager.move(from, to);

    if (m_animations.contains(to)) return false;
    auto node = m_animations.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_animations.insert(std::move(node)).inserted;
}

bool AssetMetadataManager::move(MaterialId from, MaterialId to)
{
    m_assetManager.move(from, to);

    if (m_materials.contains(to)) return false;
    auto node = m_materials.extract(from);

    if (node.empty()) return false;

    node.key() = to;

    return m_materials.insert(std::move(node)).inserted;
}

void AssetMetadataManager::add(paca::fileformats::StaticMeshRef &staticMesh)
{
    auto staticMeshAssetData
        = engine::loaders::load<paca::fileformats::StaticMesh>(staticMesh.path.c_str());

    if (!staticMeshAssetData)
    {
        ERROR("Couldn't load mesh: {}", staticMesh.path);
        return;
    }

    staticMeshAssetData->name = staticMesh.name;
    staticMeshAssetData->id = staticMesh.id;
    m_assetManager.add(*staticMeshAssetData);

    const auto it = m_staticMeshes.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(StaticMeshId(staticMesh.id)),
        std::forward_as_tuple());

    ASSERT_MSG(it.second, "Error static mesh id {} is already on assets metadata", staticMesh.id);

    StaticMeshMetadata &metadata =  it.first->second;
    metadata.name = staticMesh.name;
    metadata.path = staticMesh.path;
    metadata.preview.init({
        .width = 96,
        .height = 96,
        .format = Texture::Format::RGB8,
        .linearMinification = false,
        .linearMagnification = false,
        .interpolateBetweenMipmapLevels = false,
        .tile = false,
    });

    const StaticMesh *staticMeshAsset = m_assetManager.get(StaticMeshId(staticMesh.id));
    ASSERT(staticMeshAsset);
    m_previewRenderer.drawPreviewToTexture(staticMeshAsset, nullptr, metadata.preview, m_assetManager);
}

void AssetMetadataManager::add(paca::fileformats::AnimatedMeshRef &animatedMesh)
{
    auto animatedMeshAssetData
        = engine::loaders::load<paca::fileformats::AnimatedMesh>(animatedMesh.path.c_str());

    if (!animatedMeshAssetData)
    {
        ERROR("Couldn't load mesh: {}", animatedMesh.path);
        return;
    }

    animatedMeshAssetData->name = animatedMesh.name;
    animatedMeshAssetData->id = animatedMesh.id;
    m_assetManager.add(*animatedMeshAssetData);

    const auto it = m_animatedMeshes.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(AnimatedMeshId(animatedMesh.id)),
        std::forward_as_tuple());

    ASSERT_MSG(it.second, "Error static mesh id {} is already on assets metadata", animatedMesh.id);

    AnimatedMeshMetadata &metadata =  it.first->second;
    metadata.name = animatedMesh.name;
    metadata.path = animatedMesh.path;
    metadata.preview.init({
        .width = 96,
        .height = 96,
        .format = Texture::Format::RGB8,
        .linearMinification = false,
        .linearMagnification = false,
        .interpolateBetweenMipmapLevels = false,
        .tile = false,
    });

    const AnimatedMesh *animatedMeshAsset = m_assetManager.get(AnimatedMeshId(animatedMesh.id));
    ASSERT(animatedMeshAsset);
    //m_previewRenderer.drawPreviewToTexture(animatedMeshAsset, nullptr, metadata.preview, m_assetManager);
}

void AssetMetadataManager::add(paca::fileformats::AnimationRef &animation)
{
    auto animationAssetData
        = engine::loaders::load<paca::fileformats::Animation>(animation.path.c_str());

    if (!animationAssetData)
    {
        ERROR("Couldn't load mesh: {}", animation.path);
        return;
    }

    animationAssetData->name = animation.name;
    animationAssetData->id = animation.id;
    m_assetManager.add(*animationAssetData);

    const auto it = m_animations.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(AnimationId(animation.id)),
        std::forward_as_tuple());

    ASSERT_MSG(it.second, "Error static mesh id {} is already on assets metadata", animation.id);

    AnimationMetadata &metadata =  it.first->second;
    metadata.name = animation.name;
    metadata.path = animation.path;

    const Animation *animationAsset = m_assetManager.get(AnimationId(animation.id));
    ASSERT(animationAsset);
    //m_previewRenderer.drawPreviewToTexture(animatedMeshAsset, nullptr, metadata.preview, m_assetManager);
}

void AssetMetadataManager::add(paca::fileformats::Material &material)
{
    m_assetManager.add(material);
    const auto it = m_materials.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(MaterialId(material.id)),
        std::forward_as_tuple());

    ASSERT_MSG(it.second, "Error material id {} is already on assets metadata", material.id);

    MaterialMetadata &metadata = it.first->second;
    metadata.name = material.name;
    metadata.preview.init({
        .width = 96,
        .height = 96,
        .format = Texture::Format::RGB8,
        .linearMinification = true,
        .linearMagnification = true,
        .interpolateBetweenMipmapLevels = false,
        .tile = false,
    });
    const Material *materialAsset = m_assetManager.get(MaterialId(material.id));
    ASSERT(materialAsset);
    m_previewRenderer.drawPreviewToTexture(nullptr, materialAsset, metadata.preview, m_assetManager);
}
