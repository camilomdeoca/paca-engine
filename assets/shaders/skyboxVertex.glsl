#version 450 core

layout (location = 0) in vec3 a_position;

layout (location = 0) out vec3 o_textureCoords;

uniform mat4 u_projectionView;

void main()
{
    o_textureCoords = a_position;
    const vec4 position = u_projectionView * vec4(a_position, 1.0);
    gl_Position = position.xyww;
}
