#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

void main()
{
    float f = 0.11f;
    TexCoords = vec3(aPos.xy, -f);
    gl_Position = vec4(aPos, 1.0);
}