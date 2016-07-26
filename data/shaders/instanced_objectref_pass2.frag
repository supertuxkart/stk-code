#ifndef Use_Bindless_Texture
uniform sampler2D Albedo;
uniform sampler2D SpecMap;
#endif

#ifdef Use_Bindless_Texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
#endif
in vec2 uv;
in vec4 color;
out vec4 FragColor;

#stk_include "utils/getLightFactor.frag"

void main(void)
{
#ifdef Use_Bindless_Texture
    vec4 col = texture(handle, uv);
    float specmap = texture(secondhandle, uv).g;
#ifdef SRGBBindlessFix
    col.xyz = pow(col.xyz, vec3(2.2));
#endif
#else
    vec4 col = texture(Albedo, uv);
    float specmap = texture(SpecMap, uv).g;
#endif
    col.xyz *= pow(color.xyz, vec3(2.2));
    if (col.a * color.a < 0.5) discard;
    FragColor = vec4(getLightFactor(col.xyz, vec3(1.), specmap, 0.), 1.);
}
