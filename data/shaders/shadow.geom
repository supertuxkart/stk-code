layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

in vec2 tc[3];
in int layerId[3];

out vec2 uv;

void main(void)
{
  gl_Layer = layerId[0];
  for(int i=0; i<3; i++)
  {
    uv = tc[i];
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}
