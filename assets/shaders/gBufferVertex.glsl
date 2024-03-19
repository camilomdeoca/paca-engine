#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uvCoords;

layout (location = 0) out vec3 o_fragPosition;
layout (location = 1) out vec3 o_normal;
layout (location = 2) out vec2 o_uvCoords;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewModelMatrix;

void main()
{
    o_fragPosition = vec3(u_viewModelMatrix * vec4(a_position, 1.0)); // position in view Coords
    o_uvCoords = a_uvCoords;
    o_normal = normalize(mat3(transpose(inverse(u_viewModelMatrix))) * a_normal);
    gl_Position = u_projectionMatrix * u_viewModelMatrix * vec4(a_position, 1.0);
}

