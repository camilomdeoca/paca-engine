#version 450 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_tangent;
layout (location = 3) in vec2 a_uvCoords;

layout (location = 0) out vec2 o_uvCoords;
layout (location = 1) out mat3 o_TBN;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewModelMatrix;

void main()
{
    vec3 T = normalize(vec3(u_viewModelMatrix * vec4(a_tangent, 0.0)));
    vec3 N = normalize(vec3(u_viewModelMatrix * vec4(a_normal, 0.0)));
    vec3 B = normalize(cross(N, T));
    o_TBN = mat3(T, B, N);
    o_uvCoords = a_uvCoords;
    gl_Position = u_projectionMatrix * u_viewModelMatrix * vec4(a_position, 1.0);
}

