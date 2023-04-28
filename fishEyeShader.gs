#version 460  

layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 model;
uniform mat4 views[6];
uniform mat4 projection;

void main() 
{     
    for(int k=0; k<gl_in.length(); k++)
    {
        gl_Layer = gl_InvocationID;
        gl_Position = projection * views[gl_InvocationID] * model *  gl_in[k].gl_Position;
        EmitVertex();
    }

    EndPrimitive();
}