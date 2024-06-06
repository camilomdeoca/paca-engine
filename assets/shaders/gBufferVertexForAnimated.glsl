#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_tangent;
layout (location = 3) in vec2 a_uvCoords;
layout (location = 4) in uvec4 a_boneIds;
layout (location = 5) in vec4 a_boneWeights;

layout (location = 0) out vec2 o_uvCoords;
layout (location = 1) out vec3 o_position;
layout (location = 2) out mat3 o_TBN;
layout (location = 5) flat out uvec4 o_boneIds;
layout (location = 6) flat out vec4 o_boneWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
const uint INVALID_BONE_ID = 0xFFFFFFFF;

uniform mat4 u_finalBonesMatrices[MAX_BONES];
uniform mat4 u_projectionMatrix;
uniform mat4 u_viewModelMatrix;

void main()
{
    mat4 boneTransform = mat4(0.0);
    for (uint i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (a_boneIds[i] == INVALID_BONE_ID)
            continue;
        boneTransform += u_finalBonesMatrices[a_boneIds[i]] * a_boneWeights[i];
    }
    vec4 totalPosition = boneTransform * vec4(a_position, 1.0);
    vec3 normal  = vec3(boneTransform * vec4(a_normal, 0.0));
    vec3 tangent = vec3(boneTransform * vec4(a_tangent, 0.0));
    //vec4 totalPosition = vec4(a_position, 1.0);
    //vec3 normal  = a_normal;
    //vec3 tangent = a_tangent;

    vec3 T = normalize(vec3(u_viewModelMatrix * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(u_viewModelMatrix * vec4(normal, 0.0)));
    vec3 B = normalize(cross(N, T));
    o_TBN = mat3(T, B, N);

    o_uvCoords = a_uvCoords;
    vec4 position = u_viewModelMatrix * totalPosition;
    gl_Position = u_projectionMatrix * position;
    o_position = position.xyz/position.w;
    o_boneIds = a_boneIds;
    o_boneWeights = a_boneWeights;
}

