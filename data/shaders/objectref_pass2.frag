#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D Albedo;
layout(bindless_sampler) uniform sampler2D SpecMap;
#else
uniform sampler2D Albedo;
uniform sampler2D SpecMap;
#endif

in vec2 uv;
in vec4 color;
out vec4 FragColor;

#stk_include "utils/getLightFactor.frag"

void main(void)
{
    vec4 col = texture(Albedo, uv);
#ifdef Use_Bindless_Texture
#ifdef SRGBBindlessFix
    col.xyz = pow(col.xyz, vec3(2.2));
#endif
#endif
    col.xyz *= pow(color.xyz, vec3(2.2));
    if (col.a * color.a < 0.5) discard;
    float specmap = texture(SpecMap, uv).g;
    float emitmap = texture(SpecMap, uv).b;
    
    FragColor = vec4(getLightFactor(col.xyz, vec3(1.), specmap, emitmap), 1.);
}
