#version 450 core

layout (location = 0) in vec2 o_uvCoords;
layout (location = 1) in vec3 o_position;
layout (location = 2) in mat3 o_TBN;

layout (location = 0) out vec4 outColor;

#ifdef USE_PARALLAX_MAPPING
// To write modified depth from parallax mapping
//layout (depth_greater) out float gl_FragDepth;
uniform float u_parallaxScale;
#endif

uniform sampler2D u_diffuseMap0;
uniform sampler2D u_specularMap0;
uniform sampler2D u_normalMap0;
uniform sampler2D u_heightMap0;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewModelMatrix;

uniform bool u_hasDiffuse = false;
uniform bool u_hasSpecular = false;
uniform bool u_hasNormal = false;
uniform bool u_hasHeight = false;

struct PointLight {
    vec3 posInViewSpace;
    vec3 color;
    float intensity;
    float attenuation;
};

#define MAX_LIGHTS 10

uniform PointLight u_lights[MAX_LIGHTS];
uniform int u_numOfLights;

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

uniform DirectionalLight u_directionalLight;

#ifdef USE_SHADOW_MAPPING
uniform ShadowMapLevel u_shadowMaps[MAX_SHADOW_MAP_LEVELS];
uniform int u_numOfShadowMapLevels;
#endif // USE_SHADOW_MAPPING

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

#ifdef USE_PARALLAX_MAPPING
// https://web.archive.org/web/20141007070805/http://sunandblackcat.com/tipFullView.php?topicid=28
//
// viewDirExtension is the vector from the original texture coords at height 0 to the new texture
// coords at the height of the new texture coords (in tangent space).
// If T0 and T1 are the original and new texture coords (both are vec2), and H(T1) is the height
// at T1 then the vector is:
//                        viewDirExtension = (T1.x, T1.y, -H(T1)) - (T2.x, T2.y, 0)
//
vec2 parallaxOcclusionMapping(vec2 texCoords, vec3 viewDir, out vec3 viewDirExtension, out float parallaxHeight)
{
    // determine optimal number of layers
   const float minLayers = 8;
   const float maxLayers = 16;
   float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0, 0, 1), viewDir)));

   // height of each layer
   float layerHeight = 1.0 / numLayers;
   // current depth of the layer
   float curLayerHeight = 0;
   // shift of texture coordinates for each layer
   vec2 dtex = u_parallaxScale * viewDir.xy / viewDir.z / numLayers;

   // current texture coordinates
   vec2 currentTextureCoords = texCoords;

   // depth from heightmap
   float heightFromTexture = 1.0 - texture(u_heightMap0, currentTextureCoords).r;

   // while point is above the surface
   while(heightFromTexture > curLayerHeight)
   {
      // to the next layer
      curLayerHeight += layerHeight;
      // shift of texture coordinates
      currentTextureCoords -= dtex;
      // new depth from heightmap
      heightFromTexture = 1.0 - texture(u_heightMap0, currentTextureCoords).r;
   }

   ///////////////////////////////////////////////////////////

   // previous texture coordinates
   vec2 prevTCoords = currentTextureCoords + dtex;

   // heights for linear interpolation
   float nextH = heightFromTexture - curLayerHeight;
   float prevH = 1.0 - texture(u_heightMap0, prevTCoords).r
                           - curLayerHeight + layerHeight;

   // proportions for linear interpolation
   float weight = nextH / (nextH - prevH);

   // interpolation of texture coordinates
   vec2 finalTexCoords = prevTCoords * weight + currentTextureCoords * (1.0-weight);

   // interpolation of depth values
   parallaxHeight = curLayerHeight + prevH * weight + nextH * (1.0 - weight);
   vec2 viewDirExtensionXZ = texCoords - finalTexCoords;
   viewDirExtension = vec3(viewDirExtensionXZ.x, -parallaxHeight, viewDirExtensionXZ.y);

   // return result
   return finalTexCoords;
}
#endif

void main()
{
    vec2 textureCoords;
#ifdef USE_PARALLAX_MAPPING
    if (u_hasHeight)
    {
        vec3 viewDir = normalize(transpose(o_TBN) * o_position);
        vec3 viewDirExtension;
        float parallaxHeight;
        textureCoords = parallaxOcclusionMapping(o_uvCoords, viewDir, viewDirExtension, parallaxHeight);
        // Why does writing to gl_FragDepth disable depth testing?!?!?!?
        // I was trying to write the aparent depth offset from the parallaxMapping to the depth 
        // buffer but it does not work. 
        // I'm surely doing something wrong because im not seeing why it shouldn't work.
        //gl_FragDepth = length(u_projectionMatrix * vec4(o_TBN * viewDirExtension, 1.0));
    }
    else
        textureCoords = o_uvCoords;
#else
    textureCoords = o_uvCoords;
#endif

    vec3 color;
    vec3 normal;
    float spec;

    if (u_hasNormal)
    {
        normal = texture(u_normalMap0, textureCoords).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(o_TBN * normal);
    }
    else
        normal = o_TBN[2]; // the third column of the TBN matrix is the normal vector

    if (u_hasDiffuse)
        color = texture(u_diffuseMap0, textureCoords).rgb;
    else
        color = vec3(1.0, 0.0, 1.0); // purple as default

    if (u_hasSpecular)
        spec = texture(u_specularMap0, textureCoords).r;
    else
        spec = 1.0;

    vec3 ambient = color * 0.15;
    vec3 final = vec3(0);

    {
        const vec3 lightDir = normalize(u_directionalLight.directionInViewSpace);
        const float diffuse = max(dot(normal, -lightDir), 0.0);

        const vec3 viewDir = normalize(o_position);
        const vec3 reflectDirection = reflect(lightDir, normal);
        const float specular = pow(max(dot(-viewDir, reflectDirection), 0.0), 16) * spec;

#ifdef USE_SHADOW_MAPPING
        uint level;
        for (uint i = 0; i < u_numOfShadowMapLevels; i++)
        {
            if (abs(o_position.z) < u_shadowMaps[i].cutoffDistance)
            {
                level = i;
                break;
            }
        }
    
        //float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), SHADOW_CALCULATIONS_BIAS);
        float bias = SHADOW_CALCULATIONS_BIAS;
        bias *= 1 / (u_shadowMaps[level].cutoffDistance * 0.5f);
    
        float shadow = shadowCalculation(u_shadowMaps[level].cameraSpaceToLightSpace * vec4(o_position, 1.0), level, bias);
#endif // USE_SHADOW_MAPPING

        final += diffuse * color * u_directionalLight.intensity * u_directionalLight.color
#ifdef USE_SHADOW_MAPPING
            * (1.0 - shadow)
#endif // USE_SHADOW_MAPPING
            ;
        final += specular * u_directionalLight.color * u_directionalLight.intensity
#ifdef USE_SHADOW_MAPPING
            * (1.0 - shadow)
#endif // USE_SHADOW_MAPPING
            ;
    }

    for (int i = 0; i < u_numOfLights; i++)
    {
        PointLight light = u_lights[i];
        vec3 lightDir = normalize(o_position - light.posInViewSpace);
        float diffuse = max(dot(normal, -lightDir), 0.0);

        vec3 viewDir = normalize(o_position);
        vec3 reflectDirection = reflect(lightDir, normal);
        float specular = pow(max(dot(-viewDir, reflectDirection), 0.0), 16) * spec;

        float distance = length(light.posInViewSpace - o_position);
        float attenuation = 1.0 / (1.0 + light.attenuation*distance*distance);

        final += diffuse * color * light.intensity * light.color * attenuation;
        final += specular * light.color * light.intensity * attenuation;
    }

    outColor = vec4(final + ambient, 1.0);
}
