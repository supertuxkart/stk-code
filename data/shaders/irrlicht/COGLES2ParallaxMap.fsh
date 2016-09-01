precision mediump float;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float uHeightScale;
in vec4 varTexCoord;
in vec3 varLightVector[2];
in vec4 varLightColor[2];
in vec3 varEyeVector;
void main ()
{
  vec4 color_1;
  vec4 normalMap_2;
  normalMap_2 = (((texture (texture1, varTexCoord.xy) * 2.0) - 1.0) * uHeightScale);
  vec2 tmpvar_3;
  tmpvar_3 = ((varEyeVector.xy * normalMap_2.w) + varTexCoord.xy);
  vec4 tmpvar_4;
  tmpvar_4 = normalize(((texture (texture1, tmpvar_3) * 2.0) - 1.0));
  normalMap_2 = tmpvar_4;
  color_1 = ((clamp (varLightColor[0], 0.0, 1.0) * dot (tmpvar_4.xyz, 
    normalize(varLightVector[0])
  )) + (clamp (varLightColor[1], 0.0, 1.0) * dot (tmpvar_4.xyz, 
    normalize(varLightVector[1])
  )));
  color_1.xyz = (color_1 * texture (texture0, tmpvar_3)).xyz;
  color_1.w = varLightColor[0].w;
  gl_FragColor = color_1;
}

