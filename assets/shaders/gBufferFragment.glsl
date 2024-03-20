#version 450 core

layout (location = 0) in vec2 o_uvCoords;
layout (location = 1) in mat3 o_TBN;

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gColorSpecular;

uniform sampler2D u_diffuseMap0;
uniform sampler2D u_specularMap0;
uniform sampler2D u_normalMap0;
uniform sampler2D u_heightMap0;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewModelMatrix;

void main()
{
    vec3 normal = texture(u_normalMap0, o_uvCoords).rgb;
    normal = normal * 2.0 - 1.0;
    normal = normalize(o_TBN * normal);
    gNormal = vec4(normal, 1.0);
    gColorSpecular.rgb = texture(u_diffuseMap0, o_uvCoords).rgb;
    gColorSpecular.a = texture(u_specularMap0, o_uvCoords).r;
}

