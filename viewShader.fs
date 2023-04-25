#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

// texture sampler
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;
uniform sampler2D texture4;
uniform sampler2D texture5;
uniform sampler2D texture6;

void main()
{
    FragColor = (texture(texture1, TexCoord) + texture(texture2, TexCoord)+
    texture(texture3, TexCoord) + texture(texture4, TexCoord)+
    texture(texture5, TexCoord) + texture(texture6, TexCoord) )/6;
}