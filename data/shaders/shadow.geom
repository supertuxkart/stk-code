uniform mat4 ModelViewProjectionMatrix[4];

#if __VERSION__ >= 400
layout(triangles, invocations=4) in;
#else
layout(triangles) in;
#endif
layout(triangle_strip, max_vertices=12) out;

in vec2 tc[3];

out vec2 uv;

void emitToLayer(int layerId)
{
  gl_Layer = layerId;
  for(int i=0; i<3; i++)
  {
    uv = tc[i];
    gl_Position = ModelViewProjectionMatrix[layerId] * gl_in[i].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}

void main(void)
{
#if __VERSION__ >= 400
  emitToLayer(gl_InvocationID);
#else
  for (int j = 0; j<4; j++)
  {
    emitToLayer(j);
  }
#endif
}
