layout (location = 0) out vec2 f_uv;

void main()
{
    f_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(f_uv * 2.0 - 1.0, 1.0, 1.0);
}
