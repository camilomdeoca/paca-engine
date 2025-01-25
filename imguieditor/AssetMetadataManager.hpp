#pragma once

#include "PreviewRenderer.hpp"

#include <engine/AssetManager.hpp>
#include <engine/IdTypes.hpp>
#include "metadata/MaterialMetadata.hpp"
#include "metadata/StaticMeshMetadata.hpp"
#include "metadata/AnimatedMeshMetadata.hpp"
#include "metadata/AnimationMetadata.hpp"

#include <unordered_map>

/* When using this you add the assets to here instead of to the AssetManager and you then have
 * metada available with AssetMetadataManager::get() functions
 */
class AssetMetadataManager
{
public:
    AssetMetadataManager(AssetManager &assetManager)
        : m_assetManager(assetManager)
    {}

    /* To initialize PreviewRenderer
     * Needs to be initialized after opengl
     */
    void init();

    const StaticMeshMetadata *get(StaticMeshId id) const;
    const AnimatedMeshMetadata *get(AnimatedMeshId id) const;
    const AnimationMetadata *get(AnimationId id) const;
    const MaterialMetadata *get(MaterialId id) const;

    bool remove(StaticMeshId id);
    bool remove(AnimatedMeshId id);
    bool remove(AnimationId id);
    bool remove(MaterialId id);

    bool move(StaticMeshId from, StaticMeshId to);
    bool move(AnimatedMeshId from, AnimatedMeshId to);
    bool move(AnimationId from, AnimationId to);
    bool move(MaterialId from, MaterialId to);

    void add(paca::fileformats::StaticMeshRef &staticMesh);
    void add(paca::fileformats::AnimatedMeshRef &animatedMesh);
    void add(paca::fileformats::AnimationRef &animation);
    void add(paca::fileformats::Material &material);

    auto &staticMeshes() { return m_staticMeshes; }
    auto &animatedMeshes() { return m_animatedMeshes; }
    auto &animation() { return m_animations; }
    auto &materials() { return m_materials; }

private:
    PreviewRenderer m_previewRenderer;
    AssetManager &m_assetManager;

    // TODO: change unordered_map to a faster hashmap that has everything close in memory
    std::unordered_map<StaticMeshId, StaticMeshMetadata> m_staticMeshes;
    std::unordered_map<AnimatedMeshId, AnimatedMeshMetadata> m_animatedMeshes;
    std::unordered_map<AnimationId, AnimationMetadata> m_animations;
    std::unordered_map<MaterialId,   MaterialMetadata>   m_materials;
};

