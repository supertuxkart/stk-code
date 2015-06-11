#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform samplerCube probe;
layout(bindless_sampler) uniform sampler2D dfg;
#else
uniform samplerCube probe;
uniform sampler2D dfg;
#endif

vec3 SpecularIBL(vec3 normal, vec3 V, float roughness, vec3 F0)
{
    vec3 sampleDirection = reflect(-V, normal);
    sampleDirection = (InverseViewMatrix * vec4(sampleDirection, 0.)).xyz;
     // Assume 8 level of lod (ie 256x256 texture)

    float lodval = 7. * roughness;
    vec3 LD = max(textureLod(probe, sampleDirection, lodval).rgb, vec3(0.));

    float NdotV = clamp(dot(V, normal), 0., 1.);
    vec2 DFG = texture(dfg, vec2(NdotV, roughness)).rg;

    return LD * (F0 * DFG.x + DFG.y);
}
