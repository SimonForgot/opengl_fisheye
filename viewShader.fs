#version 460 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube fisheye_cube;

void main()
{
    float r = length(TexCoords.xy);
    if(r > 1.0f) FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0);
    else FragColor = texture(fisheye_cube, TexCoords);
}