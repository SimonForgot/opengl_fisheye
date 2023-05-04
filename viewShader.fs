#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform samplerCube fisheye_cube;

#define M_PI 3.1415926535897932384626433832795

vec4 uv_sample(vec2 uv)
{
    float len = length(uv);
    float theta = M_PI * (1.0 - 1.0 * len);
    float phi = atan(uv.y, uv.x);

    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(theta);

    if(len > 1.0f) return vec4(0.0f, 0.0f, 0.0f, 1.0);
    else return texture(fisheye_cube, vec3(x,y,z));
}

void main()
{
    vec2 uv_center = TexCoords.xy;
    vec4 c0 = uv_sample(uv_center + vec2(0.001,0.001));
    vec4 c1 = uv_sample(uv_center + vec2(-0.001,0.001));
    vec4 c2 = uv_sample(uv_center + vec2(-0.001,-0.001));
    vec4 c3 = uv_sample(uv_center + vec2(0.001,-0.001));
    vec4 c4 = uv_sample(uv_center + vec2(0.001,0.0));
    vec4 c5 = uv_sample(uv_center + vec2(0.0,0.001));
    vec4 c6 = uv_sample(uv_center + vec2(-0.001,0.0));
    vec4 c7 = uv_sample(uv_center + vec2(0.0,-0.001));
    FragColor = (uv_sample(uv_center) + c0 + c1 + c2 + c3 +c4 + c5 + c6 + c7) / 9.0f;
}