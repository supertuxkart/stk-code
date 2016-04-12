uniform samplerCube probe;

vec3 SpecularIBL(vec3 normal, vec3 V, float roughness)
{
    vec3 sampleDirection = reflect(-V, normal);
    sampleDirection = (InverseViewMatrix * vec4(sampleDirection, 0.)).xyz;

     // Assume 8 level of lod (ie 256x256 texture)
    float lodval = 7. * (1. - roughness);
    return clamp(textureLod(probe, sampleDirection, lodval).rgb, 0., 1.);
}
