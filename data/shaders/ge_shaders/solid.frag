layout(location = 0) in vec4 f_vertex_color;

layout(location = 0) out vec4 o_color;

void main()
{
    o_color = f_vertex_color;
}
