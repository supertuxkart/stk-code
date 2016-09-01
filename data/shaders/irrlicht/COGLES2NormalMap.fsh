precision mediump float;
uniform sampler2D texture0;
uniform sampler2D texture1;
in vec4 varTexCoord;
in vec3 varLightVector[2];
in vec4 varLightColor[2];
void main ()
{
  vec4 color_1;
  vec4 tmpvar_2;
  tmpvar_2 = ((texture (texture1, varTexCoord.xy) * 2.0) - 1.0);
  color_1 = ((clamp (varLightColor[0], 0.0, 1.0) * dot (tmpvar_2.xyz, 
    normalize(varLightVector[0])
  )) + (clamp (varLightColor[1], 0.0, 1.0) * dot (tmpvar_2.xyz, 
    normalize(varLightVector[1])
  )));
  color_1.xyz = (color_1 * texture (texture0, varTexCoord.xy)).xyz;
  color_1.w = varLightColor[0].w;
  gl_FragColor = color_1;
}

