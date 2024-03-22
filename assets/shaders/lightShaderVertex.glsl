#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uvCoords;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewModelMatrix;

void main()
{
    gl_Position = u_projectionMatrix * u_viewModelMatrix * vec4(a_position, 1.0);
}
