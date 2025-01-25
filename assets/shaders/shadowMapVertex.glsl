#version 450 core

layout (location = 0) in vec3 a_position;
#ifdef USE_SKINNING
layout (location = 4) in uvec4 a_boneIds;
layout (location = 5) in vec4 a_boneWeights;
#endif

#ifdef USE_SKINNING
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
const uint INVALID_BONE_ID = 0xFFFFFFFF;
uniform mat4 u_finalBonesMatrices[MAX_BONES];
#endif

uniform mat4 u_lightSpaceModelMatrix;

void main()
{
#ifdef USE_SKINNING
    mat4 boneTransform = mat4(0.0);
    for (uint i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (a_boneIds[i] == INVALID_BONE_ID)
            continue;
        boneTransform += u_finalBonesMatrices[a_boneIds[i]] * a_boneWeights[i];
    }
    vec4 position = boneTransform * vec4(a_position, 1.0);
#else
    vec4 position = vec4(a_position, 1.0);
#endif
    gl_Position = u_lightSpaceModelMatrix * position;
}
