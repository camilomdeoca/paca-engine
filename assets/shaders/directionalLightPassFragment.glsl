#version 450 core

layout (location = 0) in vec2 o_uv;

layout (location = 0) out vec4 outColor;

struct DirectionalLight {
    vec3 directionInViewSpace;
    vec3 color;
    float intensity;
};

#ifdef USE_SHADOW_MAPPING
struct ShadowMapLevel {
    sampler2D texture;
    mat4 cameraSpaceToLightSpace;
    float cutoffDistance;
};

#define MAX_SHADOW_MAP_LEVELS 5
#define SHADOW_CALCULATIONS_BIAS 0.005
#endif // USE_SHADOW_MAPPING

uniform DirectionalLight u_light;

uniform mat4 u_inverseProjectionMatrix;

uniform sampler2D u_gNormal;
uniform sampler2D u_gColorSpec;
uniform sampler2D u_gDepth;

#ifdef USE_SHADOW_MAPPING
uniform ShadowMapLevel u_shadowMaps[MAX_SHADOW_MAP_LEVELS];
uniform int u_numOfShadowMapLevels;
#endif // USE_SHADOW_MAPPING

// Get position in camera space
vec3 getPosition()
{
    float depth = texture(u_gDepth, o_uv).x * 2.0 - 1.0;
    vec4 pos = vec4(o_uv * 2.0 - 1.0, depth, 1.0);
    pos = u_inverseProjectionMatrix * pos;
    return pos.xyz / pos.w;
}

#ifdef USE_SHADOW_MAPPING
float shadowCalculation(vec4 fragPosLightSpace, uint level, float bias)
{
    vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projectedCoords = projectedCoords * 0.5 + 0.5;
    float currentDepth = projectedCoords.z;

    vec2 texelSize = 1.0 / textureSize(u_shadowMaps[level].texture, 0);
    float shadow = 0.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float closestDepth = texture(u_shadowMaps[level].texture, projectedCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projectedCoords.z > 1.0) shadow = 0.0;
    return shadow;
}
#endif // USE_SHADOW_MAPPING

void main()
{
    // Get variables from G-buffers
    const vec3 position = getPosition();
    const vec3 normal = texture(u_gNormal, o_uv).rgb;
    const vec3 color = vec4(texture(u_gColorSpec, o_uv).rgb, 1.0).rgb;
    const float spec = texture(u_gColorSpec, o_uv).a;

    vec3 ambient = color * 0.05;
    vec3 final = vec3(0.0);

    const vec3 lightDir = normalize(u_light.directionInViewSpace);
    const float diffuse = max(dot(normal, -lightDir), 0.0);

    const vec3 viewDir = normalize(position);
    const vec3 reflectDirection = reflect(lightDir, normal);
    const float specular = pow(max(dot(-viewDir, reflectDirection), 0.0), 16) * spec;

    final += diffuse * color * u_light.intensity * u_light.color;
    final += specular * u_light.color * u_light.intensity;

#ifdef USE_SHADOW_MAPPING
    uint level;
    for (uint i = 0; i < u_numOfShadowMapLevels; i++)
    {
        if (abs(position.z) < u_shadowMaps[i].cutoffDistance)
        {
            level = i;
            break;
        }
    }

    //float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), SHADOW_CALCULATIONS_BIAS);
    float bias = SHADOW_CALCULATIONS_BIAS;
    bias *= 1 / (u_shadowMaps[level].cutoffDistance * 0.5f);

    float shadow = shadowCalculation(u_shadowMaps[level].cameraSpaceToLightSpace * vec4(position, 1.0), level, bias);
    final *= (1.0 - shadow);
#endif // USE_SHADOW_MAPPING
    
#if defined(DEBUG_CASCADING_SHADOW_MAPS) && defined(USE_SHADOW_MAPPING) 
    vec3 colors[] = {
        vec3(1.0, 0.0, 0.0),
        vec3(1.0, 1.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 1.0, 1.0),
        vec3(0.0, 0.0, 1.0),
        vec3(1.0, 0.0, 1.0)
    }; 
    outColor = vec4((final + ambient)*colors[level], 1.0);
#else
    outColor = vec4((final + ambient), 1.0);
#endif
    //outColor = vec4((final + ambient) * vec3(texture(u_shadowMaps[0].texture, o_uv).r), 1.0);
    //outColor = vec4(vec3(texture(u_gDepth, o_uv).x), 1.0);
    //outColor = vec4(vec3(texture(u_gDepth, o_uv).x * 2.0 - 1.0), 1.0);
    //outColor = vec4(diffuse * color * u_lights.intensity + specular * u_lights.color * u_lights.intensity + ambient, 1.0);
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
