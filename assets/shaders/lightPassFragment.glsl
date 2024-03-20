#version 450 core

layout (location = 0) in vec2 o_uv;

layout (location = 0) out vec4 outColor;

struct Light {
    vec3 posInViewSpace;
    vec3 color;
    float intensity;
    float attenuation;
};

#define MAX_LIGHTS 10

uniform Light u_lights[MAX_LIGHTS];
uniform int u_numOfLights;

uniform mat4 u_inverseProjectionViewMatrix;

uniform sampler2D u_gNormal;
uniform sampler2D u_gColorSpec;
uniform sampler2D u_gDepth;

vec3 getPosition()
{
    float depth = texture(u_gDepth, o_uv).x * 2.0 - 1.0;
    vec4 pos = vec4(o_uv * 2.0 - 1.0, depth, 1.0);
    pos = u_inverseProjectionViewMatrix * pos;
    return pos.xyz / pos.w;
}

void main()
{
    const vec3 position = getPosition();
    const vec3 normal = texture(u_gNormal, o_uv).rgb;
    const vec3 color = vec4(texture(u_gColorSpec, o_uv).rgb, 1.0).rgb;
    const float spec = texture(u_gColorSpec, o_uv).a;

    vec3 ambient = color * 0.01;
    vec3 final = vec3(0);

    for (int i = 0; i < u_numOfLights; i++)
    {
        Light light = u_lights[i];
        vec3 lightDir = normalize(position - light.posInViewSpace);
        float diffuse = max(dot(normal, -lightDir), 0.0);

        vec3 viewDir = normalize(position);
        vec3 reflectDirection = reflect(lightDir, normal);
        float specular = pow(max(dot(-viewDir, reflectDirection), 0.0), 16) * spec;

        float distance = length(light.posInViewSpace - position);
        float attenuation = 1.0 / (1.0 + light.attenuation*distance*distance);

        final += diffuse * color * light.intensity * light.color;
        final += specular * light.color * light.intensity;
        final *= attenuation;
    }

    outColor = vec4(final + ambient, 1.0);
    //outColor = vec4(normal, 1.0);
    //outColor = vec4(diffuse * color * u_lights.intensity + specular * u_lights.color * u_lights.intensity + ambient, 1.0);
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
