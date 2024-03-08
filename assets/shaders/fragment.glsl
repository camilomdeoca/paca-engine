#version 450 core

layout (location = 0) in vec2 o_uv;

layout (location = 0) out vec4 color;

uniform vec4 u_color;
uniform sampler2D u_texture;

void main()
{
    color = texture(u_texture, o_uv) * u_color;
}

