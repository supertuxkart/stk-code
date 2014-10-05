#ifdef GL_ARB_bindless_texture
layout(bindless_sampler) uniform sampler2D DiffuseMap;
layout(bindless_sampler) uniform sampler2D SpecularMap;
layout(bindless_sampler) uniform sampler2D SSAO;
#else
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform sampler2D SSAO;
#endif

vec3 getLightFactor(vec3 diffuseMatColor, vec3 specularMatColor, float specMapValue)
{
    vec2 tc = gl_FragCoord.xy / screen;
    vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
    float ao = texture(SSAO, tc).x;
    vec3 tmp = diffuseMatColor * DiffuseComponent * (1. - specMapValue) + specularMatColor * SpecularComponent * specMapValue;
    return tmp * ao;
}