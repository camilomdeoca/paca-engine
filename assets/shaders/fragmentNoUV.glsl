#version 450 core

layout (location = 0) in vec2 o_uvCoords;
layout (location = 1) in vec3 o_normals;

layout (location = 0) out vec4 color;

uniform vec4 u_color;

void main()
{
    //color = u_color;
    color = vec4(o_normals, 1.0f);
}

