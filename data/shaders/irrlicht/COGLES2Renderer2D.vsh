in vec4 inVertexPosition;
in vec4 inVertexColor;
in vec2 inTexCoord0;
uniform mat4 uOrthoMatrix;
out vec4 vVertexColor;
out vec2 vTexCoord;
void main ()
{
  gl_Position = (uOrthoMatrix * inVertexPosition);
  vVertexColor = inVertexColor.zyxw;
  vTexCoord = inTexCoord0;
}

