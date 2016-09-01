precision mediump float;
uniform bool uUseTexture;
uniform sampler2D uTextureUnit;
in vec4 vVertexColor;
in vec2 vTexCoord;
void main ()
{
  vec4 Color_1;
  Color_1 = vVertexColor;
  if (uUseTexture) {
    Color_1 = (vVertexColor * texture (uTextureUnit, vTexCoord));
  };
  gl_FragColor = Color_1;
}

