#version 450 core

layout (location = 0) in vec2 o_uv;

layout (location = 0) out vec4 color;

uniform mat3 u_kernel;
uniform sampler2D u_screenTexture;

void main()
{
    vec2 offset = 1.0 / textureSize(u_screenTexture, 0);
    vec2 offsets[9] = vec2[](
        vec2(-offset.x,  offset.y), // top-left
        vec2(     0.0f,  offset.y), // top-center
        vec2( offset.x,  offset.y), // top-right
        vec2(-offset.x,      0.0f), // center-left
        vec2(     0.0f,      0.0f), // center-center
        vec2( offset.x,      0.0f), // center-right
        vec2(-offset.x, -offset.y), // bottom-left
        vec2(     0.0f, -offset.y), // bottom-center
        vec2( offset.x, -offset.y)  // bottom-right
    );

    vec3 sampledPixels[9];
    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            sampledPixels[j*3 + i] = vec3(texture(u_screenTexture, o_uv + offset[j*3 + i]));
        }
    }

    vec3 col = vec3(0.0);
    for (int j = 0; j < 3; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            col += sampledPixels[j*3 + i] * u_kernel[i][j];
        }
    }

    float gamma = 2.2;
    color = vec4(pow(col, vec3(1.0/gamma)), 1.0);
    //color = vec4(col, 1.0);
    //color = vec4(texture(u_screenTexture, o_uv).r);
}
