#version 330 core

in vec3 Pos;

out vec4 FragColor;

void main()
{
    FragColor = vec4(Pos, 1.0f);
}