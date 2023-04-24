#version 330 core
in vec3 Pos;

out vec4 FragColor;

void main()
{
   //if (gl_FragCoord.x<400) 
    FragColor = vec4(Pos, 1.0f);
   //else 
    //FragColor = vec4(1.0,1.0,1.0, 1.0f);
}