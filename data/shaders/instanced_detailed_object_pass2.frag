#ifndef Use_Bindless_Texture
uniform sampler2D Albedo;
uniform sampler2D Detail;
uniform sampler2D SpecMap;
#endif

#ifdef Use_Bindless_Texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
flat in sampler2D fourthhandle;
#endif
in vec2 uv;
in vec2 uv_bis;
out vec4 FragColor;

#stk_include "utils/getLightFactor.frag"

void main(void)
{
#ifdef Use_Bindless_Texture
    vec4 color = texture(handle, uv);
    float specmap = texture(secondhandle, uv).g;
#ifdef SRGBBindlessFix
    color.xyz = pow(color.xyz, vec3(2.2));
#endif
    vec4 detail = texture(fourthhandle, uv_bis);
#else
    vec4 color = texture(Albedo, uv);
    vec4 detail = texture(Detail, uv_bis);
    float specmap = texture(SpecMap, uv).g;
#endif
    detail.rgb = detail.a * detail.rgb;
    color.rgb = detail.rgb + color.rgb * (1. - detail.a);
    FragColor = vec4(getLightFactor(color.xyz, vec3(1.), specmap, 0.), 1.);
}
