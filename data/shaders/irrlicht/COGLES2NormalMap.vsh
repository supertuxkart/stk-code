in vec4 inVertexPosition;
in vec4 inVertexColor;
in vec4 inTexCoord0;
in vec3 inVertexNormal;
in vec3 inVertexTangent;
in vec3 inVertexBinormal;
uniform mat4 uMvpMatrix;
uniform vec4 uLightPos[2];
uniform vec4 uLightColor[2];
out vec4 varTexCoord;
out vec3 varLightVector[2];
out vec4 varLightColor[2];
out vec4 debug;
void main ()
{
  vec4 tmpvar_1;
  tmpvar_1.w = 1.0;
  tmpvar_1.xyz = inVertexNormal;
  debug = tmpvar_1;
  gl_Position = (uMvpMatrix * inVertexPosition);
  vec4 tmpvar_2;
  tmpvar_2 = (uLightPos[0] - inVertexPosition);
  vec4 tmpvar_3;
  tmpvar_3 = (uLightPos[1] - inVertexPosition);
  varLightVector[0].x = dot (inVertexTangent, tmpvar_2.xyz);
  varLightVector[0].y = dot (inVertexBinormal, tmpvar_2.xyz);
  varLightVector[0].z = dot (inVertexNormal, tmpvar_2.xyz);
  varLightVector[1].x = dot (inVertexTangent, tmpvar_3.xyz);
  varLightVector[1].y = dot (inVertexBinormal, tmpvar_3.xyz);
  varLightVector[1].z = dot (inVertexNormal, tmpvar_3.xyz);
  varLightColor[0].w = 0.0;
  varLightColor[0].x = dot (tmpvar_2, tmpvar_2);
  varLightColor[0].x = (varLightColor[0].x * uLightColor[0].w);
  varLightColor[0] = vec4(inversesqrt(varLightColor[0].x));
  varLightColor[0] = (varLightColor[0] * uLightColor[0]);
  varLightVector[0] = normalize(varLightVector[0]);
  varLightColor[1].w = 0.0;
  varLightColor[1].x = dot (tmpvar_3, tmpvar_3);
  varLightColor[1].x = (varLightColor[1].x * uLightColor[1].w);
  varLightColor[1] = vec4(inversesqrt(varLightColor[1].x));
  varLightColor[1] = (varLightColor[1] * uLightColor[1]);
  varLightVector[1] = normalize(varLightVector[1]);
  varTexCoord = inTexCoord0;
  varLightColor[0].w = inVertexColor.w;
}

