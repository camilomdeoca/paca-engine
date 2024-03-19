#version 450 core

layout (location = 0) in vec3 o_fragPosition;
layout (location = 1) in vec3 o_normal;
layout (location = 2) in vec2 o_uvCoords;

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gColorSpecular;

uniform sampler2D u_diffuseMap0;
uniform sampler2D u_specularMap0;
uniform sampler2D u_normalMap0;

void main()
{
    gNormal = vec4(o_normal, 1.0);
    gColorSpecular.rgb = texture(u_diffuseMap0, o_uvCoords).rgb;
    gColorSpecular.a = texture(u_specularMap0, o_uvCoords).r;
    //gColorSpecular.a = 1.0;
}

