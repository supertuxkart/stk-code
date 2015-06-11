// Fresnel Schlick approximation
vec3 Fresnel(vec3 viewdir, vec3 halfdir, vec3 basecolor)
{
    return clamp(basecolor + (1. - basecolor) * pow(1. - clamp(dot(viewdir, halfdir), 0., 1.), 5.), vec3(0.), vec3(1.));
}


// Schlick geometry term
float G1(vec3 V, vec3 normal, float k)
{
    float NdotV = clamp(dot(V, normal), 0., 1.);
    return 1. / (NdotV * (1. - k) + k);
}

// Smith model
// We factor the (n.v) (n.l) factor in the denominator
float ReducedGeometric(vec3 lightdir, vec3 viewdir, vec3 normal, float roughness)
{
    float k = (roughness + 1.) * (roughness + 1.) / 8.;
    return G1(lightdir, normal, k) * G1(viewdir, normal, k);
}

// GGX
float Distribution(float roughness, vec3 normal, vec3 halfdir)
{
    float alpha = roughness * roughness * roughness * roughness;
    float NdotH = clamp(dot(normal, halfdir), 0., 1.);
    float normalisationFactor = 1. / 3.14;
    float denominator = NdotH * NdotH * (alpha - 1.) + 1.;
    return normalisationFactor * alpha / (denominator * denominator);
}

vec3 SpecularBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness)
{
    // Half Light View direction
    vec3 H = normalize(eyedir + lightdir);
    float biaised_roughness = 0.05 + 0.95 * roughness;

    // Microfacet model
    return Fresnel(eyedir, H, color) * ReducedGeometric(lightdir, eyedir, normal, biaised_roughness) * Distribution(biaised_roughness, normal, H) / 4.;
}
