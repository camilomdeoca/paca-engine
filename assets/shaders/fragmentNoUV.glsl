#version 450 core

layout (location = 0) in vec3 o_fragPosition;
layout (location = 1) in vec2 o_uvCoords;
layout (location = 2) in vec3 o_normal;

layout (location = 0) out vec4 color;

uniform vec4 u_color;
uniform vec3 u_viewPosition;
uniform mat4 u_viewModelMatrix;
//uniform vec3 u_lightPosInViewSpace;

void main()
{
    vec3 u_lightPosInViewSpace = vec3(u_viewModelMatrix * vec4(10.0, 12.0, -10.0, 1.0));
    vec3 u_lightColor = vec3(1.0, 1.0, 1.0);

    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * u_lightColor;

    vec3 normal = normalize(o_normal);
    vec3 lightDir = normalize(o_fragPosition - u_lightPosInViewSpace);
    vec3 diffuse = max(dot(normal, -lightDir), 0.0) * u_lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(o_fragPosition);
    vec3 reflectDirection = reflect(lightDir, normal);
    float spec = pow(max(dot(-viewDir, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * spec * u_lightColor;

    color = vec4(ambient + diffuse + specular, 1.0) * u_color;
    //color = vec4(o_normal, 1.0f);
}

