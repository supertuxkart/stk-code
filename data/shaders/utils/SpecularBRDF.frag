// Blinn Phong with emulated fresnel factor
vec3 SpecularBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness)
{
    float exponentroughness = exp2(10. * roughness + 1.);
    // Half Light View direction
    vec3 H = normalize(eyedir + lightdir);
    float NdotH = max(0., dot(normal, H));
    float normalisationFactor = (exponentroughness + 2.) / 8.;
    vec3 FresnelSchlick = color + (1.0f - color) * pow(1.0f - max(0., (dot(eyedir, H))), 5);
    return max(pow(NdotH, exponentroughness) * FresnelSchlick * normalisationFactor, vec3(0.));
}