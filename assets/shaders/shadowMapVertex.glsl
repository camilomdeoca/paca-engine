#version 450 core

layout (location = 0) in vec3 a_position;

uniform mat4 u_lightSpaceModelMatrix;

void main()
{
    gl_Position = u_lightSpaceModelMatrix * vec4(a_position, 1.0);
}
