#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D tex;
#else
uniform sampler2D tex;
#endif


uniform float fogmax;
uniform float startH;
uniform float endH;
uniform float start;
uniform float end;
uniform vec3 col;

in vec2 uv;
in vec4 color;
out vec4 FragColor;


void main()
{
    vec4 diffusecolor = texture(tex, uv);
#ifdef Use_Bindless_Texture
#ifdef SRGBBindlessFix
    diffusecolor.xyz = pow(diffusecolor.xyz, vec3(2.2));
#endif
#endif
    diffusecolor.xyz *= pow(color.xyz, vec3(2.2));
    diffusecolor.a *= color.a;
    vec3 tmp = vec3(gl_FragCoord.xy / screen, gl_FragCoord.z);
    tmp = 2. * tmp - 1.;

    vec4 xpos = vec4(tmp, 1.0);
    xpos = InverseProjectionMatrix * xpos;
    xpos.xyz /= xpos.w;

    float dist = length(xpos.xyz);
    float fog = smoothstep(start, end, dist);

    fog = min(fog, fogmax);

    vec4 finalcolor = vec4(col, 0.) * fog + diffusecolor *(1. - fog);
    FragColor = vec4(finalcolor.rgb * finalcolor.a, finalcolor.a);
    //FragColor = vec4(1.0, 0.0, 0.0, finalcolor.a);
}
