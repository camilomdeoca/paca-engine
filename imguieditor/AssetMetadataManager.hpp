#pragma once

#include "PreviewRenderer.hpp"

#include <engine/AssetManager.hpp>
#include <engine/IdTypes.hpp>
#include <engine/assets/metadata/MaterialMetadata.hpp>
#include <engine/assets/metadata/StaticMeshMetadata.hpp>

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
    const MaterialMetadata *get(MaterialId id) const;

    bool remove(StaticMeshId id);
    bool remove(MaterialId id);

    bool move(StaticMeshId from, StaticMeshId to);
    bool move(MaterialId from, MaterialId to);

    void add(paca::fileformats::StaticMeshRef &staticMesh);
    void add(paca::fileformats::Material &material);

    auto &staticMeshes() { return m_staticMeshes; }
    auto &materials() { return m_materials; }

private:
    PreviewRenderer m_previewRenderer;
    AssetManager &m_assetManager;

    // TODO: change unordered_map to a faster hashmap that has everything close in memory
    std::unordered_map<StaticMeshId, StaticMeshMetadata> m_staticMeshes;
    std::unordered_map<MaterialId,   MaterialMetadata>   m_materials;
};

