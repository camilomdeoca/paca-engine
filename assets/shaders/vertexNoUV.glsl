#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uvCoords;
layout (location = 2) in vec3 a_normals;

layout (location = 0) out vec2 o_uvCoords;
layout (location = 1) out vec3 o_normals;

uniform mat4 u_viewProjection;

void main()
{
    o_uvCoords = a_uvCoords;
    o_normals = a_normals;
    gl_Position = u_viewProjection * vec4(a_position, 1.0f);
}
