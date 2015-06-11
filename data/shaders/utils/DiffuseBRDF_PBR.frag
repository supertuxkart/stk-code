// Burley model from Frostbite going pbr paper
vec3 DiffuseBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness)
{
    float biaised_roughness = 0.05 + 0.95 * roughness;
    // Half Light View direction
    vec3 H = normalize(eyedir + lightdir);
    float LdotH = clamp(dot(lightdir, H), 0., 1.);
    float NdotL = clamp(dot(lightdir, normal), 0., 1.);
    float NdotV = clamp(dot(lightdir, eyedir), 0., 1.);

    float Fd90 = 0.5 + 2 * LdotH * LdotH * biaised_roughness * biaised_roughness;
    float SchlickFresnelL = (1. + (Fd90 - 1.) * (1. - pow(NdotL, 5.)));
    float SchlickFresnelV = (1. + (Fd90 - 1.) * (1. - pow(NdotV, 5.)));
    return color * SchlickFresnelL * SchlickFresnelV / 3.14;
}
