#version 450 core

layout (location = 0) in vec3 o_fragPosition;
layout (location = 1) in vec3 o_normal;
layout (location = 2) in vec2 o_uvCoords;

layout (location = 0) out vec4 color;

uniform sampler2D u_diffuseMap0;
uniform sampler2D u_specularMap0;
uniform sampler2D u_normalMap0;

uniform mat4 u_viewModelMatrix;
//uniform vec3 u_lightPosInViewSpace;

void main()
{
    vec3 u_lightPosInViewSpace = vec3(u_viewModelMatrix * vec4(10.0, 12.0, -10.0, 1.0));
    vec3 u_lightColor = vec3(1.0, 1.0, 1.0);

    vec3 ambient = texture(u_diffuseMap0, o_uvCoords).rgb * u_lightColor;

    vec3 normal = normalize(o_normal);
    //vec3 normal = normalize(texture(u_normalMap0, o_uvCoords).rgb * 2.0 - 1.0);
    vec3 lightDir = normalize(o_fragPosition - u_lightPosInViewSpace);
    float diff = dot(normal, -lightDir);
    vec3 diffuse = max(diff, 0.0) * u_lightColor * texture(u_diffuseMap0, o_uvCoords).rgb;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(o_fragPosition);
    vec3 reflectDirection = reflect(lightDir, normal);
    float spec = pow(max(dot(-viewDir, reflectDirection), 0.0), 32);
    vec3 specular = texture(u_specularMap0, o_uvCoords).rgb * spec * u_lightColor;

    color = vec4(ambient + diffuse + specular, 1.0);
    //color = vec4(1.0);
    //color = vec4(o_normal, 1.0f);
}

