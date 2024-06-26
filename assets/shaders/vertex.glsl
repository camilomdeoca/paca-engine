#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uv;

layout (location = 0) out vec2 o_uv;

uniform mat4 u_viewProjection;

void main()
{
    o_uv = a_uv;
    gl_Position = u_viewProjection * vec4(a_position, 1.0f);
}
