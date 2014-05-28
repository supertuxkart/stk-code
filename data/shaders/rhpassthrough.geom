layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in int layer[3];
in vec2 uv_in[3];
flat out int slice;
out vec2 uv;

void main(void)
{
    gl_Layer = layer[0];
    for(int i=0; i<3; i++)
    {
        slice = layer[0];
        uv = uv_in[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
