#version 460 core

//in vec3 Pos;
//in int gl_Layer;

out vec4 FragColor;

void main()
{
    if(gl_Layer==0)
    FragColor = vec4(1.f, 0.f, 0.f, 1.0f);
    else if(gl_Layer==1)
    FragColor = vec4(0.f, 1.f, 0.f, 1.0f);
    else if(gl_Layer==2)
    FragColor = vec4(0.f, 0.f, 1.f, 1.0f);
    else if(gl_Layer==3)
    FragColor = vec4(0.f, 0.f, 0.f, 1.0f);
    else if(gl_Layer==4)
    FragColor = vec4(1.f, 1.f, 0.f, 1.0f);
    else if(gl_Layer==5)
    FragColor = vec4(1.f, 0.f, 1.f, 1.0f);
}