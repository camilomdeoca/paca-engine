#version 450 core

layout (location = 0) in vec2 o_uvCoords;
layout (location = 1) in vec3 o_position;
layout (location = 2) in mat3 o_TBN;
layout (location = 5) flat in uvec4 o_boneIds;
layout (location = 6) flat in vec4 o_boneWeights;

layout (location = 0) out vec4 gNormal;
layout (location = 1) out vec4 gColorSpecular;

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

#ifdef VISUALIZE_BONE_INFLUENCES
uniform uint u_displayBoneIndex;
#endif

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

    if (u_hasNormal)
    {
        vec3 normal = texture(u_normalMap0, textureCoords).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(o_TBN * normal);
        gNormal.xyz = normal;
    }
    else
        gNormal.xyz = o_TBN[2]; // the third column of the TBN matrix is the normal vector

    if (u_hasDiffuse)
    {
        gColorSpecular.rgb = texture(u_diffuseMap0, textureCoords).rgb;
    }
    else
        gColorSpecular.rgb = vec3(1.0, 0.0, 1.0); // purple as default

#ifdef VISUALIZE_BONE_INFLUENCES
    bool found = false;
    for (int i = 0 ; i < 4 ; i++) {
        if (o_boneIds[i] == u_displayBoneIndex) {
           if (o_boneWeights[i] >= 0.7) {
               gColorSpecular.rgb = vec3(1.0, 0.0, 0.0) * o_boneWeights[i];
           } else if (o_boneWeights[i] >= 0.4 && o_boneWeights[i] <= 0.6) {
               gColorSpecular.rgb = vec3(0.0, 1.0, 0.0) * o_boneWeights[i];
           } else if (o_boneWeights[i] >= 0.1) {
               gColorSpecular.rgb = vec3(1.0, 1.0, 0.0) * o_boneWeights[i];
           }

           found = true;
           break;
        }
    }
    if (!found)
        gColorSpecular.rgb = vec3(0.0, 0.0, 1.0);
#endif

    if (u_hasSpecular)
        gColorSpecular.a = texture(u_specularMap0, textureCoords).r;
    else
        gColorSpecular.a = 1.0;
}

