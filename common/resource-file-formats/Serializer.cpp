#include "include/Serializer.hpp"
#include "include/ResourceFileFormats.hpp"

namespace paca::fileformats {

void Serializer::operator()(const glm::quat &object)
{
    (*this)(object.x);
    (*this)(object.y);
    (*this)(object.z);
    (*this)(object.w);
}

void Unserializer::operator()(glm::quat &object)
{
    (*this)(object.x);
    (*this)(object.y);
    (*this)(object.z);
    (*this)(object.w);
}

void Serializer::operator()(const glm::vec3 &object)
{
    (*this)(object.x);
    (*this)(object.y);
    (*this)(object.z);
}

void Unserializer::operator()(glm::vec3 &object)
{
    (*this)(object.x);
    (*this)(object.y);
    (*this)(object.z);
}

void Serializer::operator()(const glm::mat4 &object)
{
    for (int j = 0; j < object.length(); j++)
        for (int i = 0; i < object[j].length(); i++)
            (*this)(object[j][i]);
}

void Unserializer::operator()(glm::mat4 &object)
{
    for (int j = 0; j < object.length(); j++)
        for (int i = 0; i < object[j].length(); i++)
            (*this)(object[j][i]);
}

void Serializer::operator()(const paca::fileformats::Bone &object)
{
    (*this)(object.parentID);
    (*this)(object.offsetMatrix);
}

void Unserializer::operator()(paca::fileformats::Bone &object)
{
    (*this)(object.parentID);
    (*this)(object.offsetMatrix);
}

void Serializer::operator()(const paca::fileformats::Skeleton &object)
{
    (*this)(object.bones);
    (*this)(object.boneNames);
}

void Unserializer::operator()(paca::fileformats::Skeleton &object)
{
    (*this)(object.bones);
    (*this)(object.boneNames);
}

void Serializer::operator()(const paca::fileformats::Mesh &object)
{
    (*this)(object.vertexType);
    (*this)(object.indexType);
    (*this)(object.vertices);
    (*this)(object.indices);
    (*this)(object.materialName);
    (*this)(object.animations);
    (*this)(object.skeleton);
}

void Unserializer::operator()(paca::fileformats::Mesh &object)
{
    (*this)(object.vertexType);
    (*this)(object.indexType);
    (*this)(object.vertices);
    (*this)(object.indices);
    (*this)(object.materialName);
    (*this)(object.animations);
    (*this)(object.skeleton);
}

void Serializer::operator()(const paca::fileformats::Model &object)
{
    (*this)(object.meshes);
    (*this)(object.name);
}

void Unserializer::operator()(paca::fileformats::Model &object)
{
    (*this)(object.meshes);
    (*this)(object.name);
}

void Serializer::operator()(const paca::fileformats::PositionKeyFrame &object)
{
    (*this)(object.time);
    (*this)(object.position);
}

void Unserializer::operator()(paca::fileformats::PositionKeyFrame &object)
{
    (*this)(object.time);
    (*this)(object.position);
}

void Serializer::operator()(const paca::fileformats::RotationKeyFrame &object)
{
    (*this)(object.time);
    (*this)(object.quaternion);
}

void Unserializer::operator()(paca::fileformats::RotationKeyFrame &object)
{
    (*this)(object.time);
    (*this)(object.quaternion);
}

void Serializer::operator()(const paca::fileformats::ScaleKeyFrame &object)
{
    (*this)(object.time);
    (*this)(object.scale);
}

void Unserializer::operator()(paca::fileformats::ScaleKeyFrame &object)
{
    (*this)(object.time);
    (*this)(object.scale);
}

void Serializer::operator()(const paca::fileformats::BoneKeyFrames &object)
{
    (*this)(object.positions);
    (*this)(object.rotations);
    (*this)(object.scalings);
}

void Unserializer::operator()(paca::fileformats::BoneKeyFrames &object)
{
    (*this)(object.positions);
    (*this)(object.rotations);
    (*this)(object.scalings);
}

void Serializer::operator()(const paca::fileformats::Animation &object)
{
    (*this)(object.duration);
    (*this)(object.ticksPerSecond);
    (*this)(object.name);
    (*this)(object.keyframes);
}

void Unserializer::operator()(paca::fileformats::Animation &object)
{
    (*this)(object.duration);
    (*this)(object.ticksPerSecond);
    (*this)(object.name);
    (*this)(object.keyframes);
}

void Serializer::operator()(const paca::fileformats::Texture &object)
{
    (*this)(object.width);
    (*this)(object.height);
    (*this)(object.channels);
    (*this)(object.isCubeMap);
    (*this)(object.pixelData);
}

void Unserializer::operator()(paca::fileformats::Texture &object)
{
    (*this)(object.width);
    (*this)(object.height);
    (*this)(object.channels);
    (*this)(object.isCubeMap);
    (*this)(object.pixelData);
}

void Serializer::operator()(const paca::fileformats::Material &object)
{
    (*this)(object.textures);
    (*this)(object.name);
}

void Unserializer::operator()(paca::fileformats::Material &object)
{
    (*this)(object.textures);
    (*this)(object.name);
}

void Serializer::operator()(const paca::fileformats::ResourcePack &object)
{
    (*this)(object.models);
    (*this)(object.materials);
    (*this)(object.animations);
}

void Unserializer::operator()(paca::fileformats::ResourcePack &object)
{
    (*this)(object.models);
    (*this)(object.materials);
    (*this)(object.animations);
}

} // namespace paca::fileformats
