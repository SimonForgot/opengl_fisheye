#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform samplerCube fisheye_cube;

#define M_PI 3.1415926535897932384626433832795

void main()
{
    float len = length(TexCoords.xy);
    float theta = M_PI * (1.0 - len);
    float phi = atan(TexCoords.y, TexCoords.x);

    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    if(len > 1.0f) FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0);
    else FragColor = texture(fisheye_cube, vec3(x,y,z));
}