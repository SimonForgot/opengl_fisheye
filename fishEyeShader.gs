#version 460  

layout(triangles, invocations = 6) in;
layout(triangle_strip, max_vertices = 3) out;

//out int gl_Layer;

void main() 
{     
    const vec2 vert_data[3] = vec2[]( vec2(-1.0, 1.0), vec2(-1.0, -1.0), vec2(1.0, 1.0));

    for(int k=0; k<gl_in.length(); k++)
    {
        gl_Layer = gl_InvocationID;
        gl_Position = vec4(vert_data[k].xy,0,1);
        EmitVertex();
    }

    EndPrimitive();
}