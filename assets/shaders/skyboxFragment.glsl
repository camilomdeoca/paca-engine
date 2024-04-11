#version 450 core

layout (location = 0) in vec3 o_textureCoords;

out vec4 outColor;

uniform samplerCube u_skybox;

void main()
{
    outColor = texture(u_skybox, o_textureCoords);
    //outColor = vec4(1.0, 0.0, 0.0, 1.0);
}

