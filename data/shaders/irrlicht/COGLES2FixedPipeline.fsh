precision mediump float;
uniform int uMaterialType;
uniform bool uTextureUsage0;
uniform sampler2D uTextureUnit0;
uniform sampler2D uTextureUnit1;
in vec2 varTexCoord0;
in vec2 varTexCoord1;
in vec4 varVertexColor;
void main ()
{
  if ((uMaterialType == 0)) {
    vec4 Color_1;
    Color_1 = varVertexColor;
    if (uTextureUsage0) {
      Color_1 = (varVertexColor * texture (uTextureUnit0, varTexCoord0));
    };
    Color_1.w = 1.0;
    gl_FragColor = Color_1;
  } else {
    if ((uMaterialType == 1)) {
      gl_FragColor = ((texture (uTextureUnit0, varTexCoord0) * varVertexColor.w) + (texture (uTextureUnit1, varTexCoord1) * (1.0 - varVertexColor.w)));
    } else {
      if ((uMaterialType == 2)) {
        vec4 Color_2;
        vec4 tmpvar_3;
        tmpvar_3 = texture (uTextureUnit0, varTexCoord0);
        Color_2.xyz = ((tmpvar_3 * texture (uTextureUnit1, varTexCoord1)) * 4.0).xyz;
        Color_2.w = (tmpvar_3.w * tmpvar_3.w);
        gl_FragColor = Color_2;
      } else {
        if ((uMaterialType == 3)) {
          gl_FragColor = (texture (uTextureUnit0, varTexCoord0) + (texture (uTextureUnit1, varTexCoord1) - 0.5));
        } else {
          if ((uMaterialType == 4)) {
            vec4 Color_4;
            Color_4 = varVertexColor;
            if (uTextureUsage0) {
              Color_4 = (varVertexColor * texture (uTextureUnit0, varTexCoord0));
            };
            Color_4.w = 1.0;
            gl_FragColor = Color_4;
          } else {
            if ((uMaterialType == 5)) {
              gl_FragColor = (varVertexColor * (texture (uTextureUnit0, varTexCoord0) * texture (uTextureUnit1, varTexCoord1)));
            } else {
              if ((uMaterialType == 6)) {
                vec4 Color_5;
                Color_5 = vec4(1.0, 1.0, 1.0, 1.0);
                if (uTextureUsage0) {
                  Color_5 = texture (uTextureUnit0, varTexCoord0);
                };
                gl_FragColor = Color_5;
              } else {
                if ((uMaterialType == 7)) {
                  vec4 Color_6;
                  Color_6 = vec4(1.0, 1.0, 1.0, 1.0);
                  if (uTextureUsage0) {
                    Color_6 = texture (uTextureUnit0, varTexCoord0);
                  };
                  if ((Color_6.w < 0.5)) {
                    discard;
                  };
                  gl_FragColor = Color_6;
                } else {
                  if ((uMaterialType == 8)) {
                    vec4 Color_7;
                    vec4 Color_8;
                    Color_8 = vec4(1.0, 1.0, 1.0, 1.0);
                    if (uTextureUsage0) {
                      Color_8 = texture (uTextureUnit0, varTexCoord0);
                    };
                    Color_7.xyz = Color_8.xyz;
                    Color_7.w = varVertexColor.w;
                    gl_FragColor = Color_7;
                  } else {
                    if ((uMaterialType == 9)) {
                      vec4 Color_9;
                      Color_9.xyz = (varVertexColor * (texture (uTextureUnit0, varTexCoord0) * texture (uTextureUnit1, varTexCoord1))).xyz;
                      Color_9.w = varVertexColor.w;
                      gl_FragColor = Color_9;
                    } else {
                      gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
                    };
                  };
                };
              };
            };
          };
        };
      };
    };
  };
}

