#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D DiffuseMap;
layout(bindless_sampler) uniform sampler2D SpecularMap;
layout(bindless_sampler) uniform sampler2D SSAO;
#else
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform sampler2D SSAO;
#endif

vec3 getLightFactor(vec3 diffuseMatColor, vec3 specularMatColor, float specMapValue, float emitMapValue)
{
#if defined(Advanced_Lighting_Enabled)
    vec2 tc = gl_FragCoord.xy / screen;
    vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
    float ao = texture(SSAO, tc).x;
    vec3 tmp = diffuseMatColor * DiffuseComponent * (1. - specMapValue) + specularMatColor * SpecularComponent * specMapValue;
    vec3 emitCol = diffuseMatColor.xyz * diffuseMatColor.xyz * diffuseMatColor.xyz * 15.;
    return tmp * ao + (emitMapValue * emitCol);
#else
#if !defined(sRGB_Framebuffer_Usable)
    return diffuseMatColor * 0.73; // 0.5 ^ (1. / 2.2)
#else
    return diffuseMatColor * 0.5;
#endif
#endif
}
