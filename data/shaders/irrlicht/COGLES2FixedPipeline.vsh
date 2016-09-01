in vec3 inVertexPosition;
in vec4 inVertexColor;
in vec2 inTexCoord0;
in vec2 inTexCoord1;
uniform mat4 uMvpMatrix;
uniform mat4 uTextureMatrix0;
uniform mat4 uTextureMatrix1;
out vec2 varTexCoord0;
out vec2 varTexCoord1;
out vec4 varVertexColor;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = inVertexPosition;
  gl_Position = (uMvpMatrix * tmpvar_1);
  vec4 tmpvar_2;
  tmpvar_2.zw = vec2(0.0, 0.0);
  tmpvar_2.xy = inTexCoord0;
  varTexCoord0 = (uTextureMatrix0 * tmpvar_2).xy;
  vec4 tmpvar_3;
  tmpvar_3.zw = vec2(0.0, 0.0);
  tmpvar_3.xy = inTexCoord1;
  varTexCoord1 = (uTextureMatrix1 * tmpvar_3).xy;
  varVertexColor = inVertexColor.zyxw;
}

