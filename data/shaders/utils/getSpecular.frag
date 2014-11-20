

vec3 getSpecular(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness)
{
    // Half Light View direction
    vec3 f0 = vec3(0.040);
    vec3 H = normalize(eyedir + lightdir);
    float NdotH = max(0., dot(normal, H));
    float normalisationFactor = (roughness + 2.) / 8.;
    vec3 FresnelSchlick = f0 + (1.0f - f0) * pow(1.0f - max(0., (dot(eyedir, H))), 5);
    return max(pow(NdotH, roughness) * FresnelSchlick * normalisationFactor, vec3(0.));
}