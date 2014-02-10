#version 330 core
uniform mat4 ModelViewProjectionMatrix[3];

layout(triangles) in;
layout(triangle_strip, max_vertices=9) out;

in vec2 tc[3];

out vec2 uv;

void main(void)
{
  for (int j = 0; j<3; j++)
  {
    gl_Layer = j;
    for(int i=0; i<3; i++)
    {
      uv = tc[i];
      gl_Position = ModelViewProjectionMatrix[j] * gl_in[i].gl_Position;
      EmitVertex();
    }
    EndPrimitive();
  }
}
