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
    for (paca::fileformats::AssetPack &resourcePack : resourcePacks)
    {
        for (const paca::fileformats::Model &model : resourcePack.models)
            result.models.emplace_back(model);
        for (const paca::fileformats::Material &material : resourcePack.materials)
            result.materials.emplace_back(material);
        for (const paca::fileformats::Texture &texture : resourcePack.textures)
            result.textures.emplace_back(texture);
        for (const paca::fileformats::Animation &animation : resourcePack.animations)
            result.animations.emplace_back(animation);
    }
    return result;
}

} // namespace paca::fileformats
