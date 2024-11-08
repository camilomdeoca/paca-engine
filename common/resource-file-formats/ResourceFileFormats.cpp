#include "include/ResourceFileFormats.hpp"

namespace paca::fileformats {

size_t vertexTypeToSize(VertexType type)
{
    switch (type) {
        case VertexType::float3pos_float3norm_float2texture:            return (3+3+2)*sizeof(float);
        case VertexType::float3pos_float3norm_float3tang_float2texture: return (3+3+3+2)*sizeof(float);
        case VertexType::float3pos_float3norm_float3tang_float2texture_int4boneIds_float4boneWeights: return (3+3+3+2)*sizeof(float) + 4*sizeof(int32_t) + 4*sizeof(float);
        default: break;
    }

    exit(1);
}

size_t indexTypeToSize(IndexType type)
{
    switch (type) {
        case IndexType::no_indices: return 0;
        case IndexType::uint32bit:   return sizeof(uint32_t);
        default: break;
    }

    exit(1);
}

paca::fileformats::AssetPack combine(std::vector<std::reference_wrapper<paca::fileformats::AssetPack>> resourcePacks)
{
    paca::fileformats::AssetPack result;

    auto append = [](auto &dest, const auto &orig) {
        dest.insert(dest.end(), orig.begin(), orig.end());
    };

    for (paca::fileformats::AssetPack &resourcePack : resourcePacks)
    {
        append(result.models, resourcePack.models);
        append(result.materials, resourcePack.materials);
        append(result.textures, resourcePack.textures);
        append(result.animations, resourcePack.animations);
        append(result.fonts, resourcePack.fonts);
    }
    return result;
}

} // namespace paca::fileformats
