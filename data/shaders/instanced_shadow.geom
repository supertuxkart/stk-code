layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

#ifdef GL_ARB_bindless_texture
flat in uvec2 hdle[3];
#endif
in vec2 tc[3];
in int layerId[3];

out vec2 uv;
#ifdef GL_ARB_bindless_texture
out flat uvec2 handle;
#endif

void main(void)
{
  gl_Layer = layerId[0];
#ifdef GL_ARB_bindless_texture
  handle = hdle[0];
#endif
  for(int i=0; i<3; i++)
  {
    uv = tc[i];
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}
