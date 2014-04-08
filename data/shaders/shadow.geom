layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
};

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
    gl_Position = ShadowViewProjMatrixes[layerId] * gl_in[i].gl_Position;
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
