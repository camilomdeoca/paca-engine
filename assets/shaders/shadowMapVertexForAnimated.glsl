#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_tangent;
layout (location = 3) in vec2 a_uvCoords;
layout (location = 4) in uvec4 a_boneIds;
layout (location = 5) in vec4 a_boneWeights;

uniform mat4 u_lightSpaceModelMatrix;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
const uint INVALID_BONE_ID = 0xFFFFFFFF;

uniform mat4 u_finalBonesMatrices[MAX_BONES];

void main()
{
    mat4 boneTransform = mat4(0.0);
    for (uint i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (a_boneIds[i] == INVALID_BONE_ID)
            continue;
        boneTransform += u_finalBonesMatrices[a_boneIds[i]] * a_boneWeights[i];
    }
    gl_Position = u_lightSpaceModelMatrix * boneTransform * vec4(a_position, 1.0);
}
