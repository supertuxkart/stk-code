vec2 F_AB(float perceptual_roughness, float NdotV)
{
    vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = perceptual_roughness * c0 + c1;
    float a004 = min(r.x * r.x, pow(2.0, -9.28 * NdotV)) * r.x + r.y;
    return vec2(-1.04, 1.04) * a004 + r.zw;
}

// Lambert model
float F_Schlick(float f0, float f90, float VdotH)
{
    return mix(f0, f90, pow(1.0 - VdotH, 5.0));
}

float Fd_Burley(float roughness, float NdotV, float NdotL, float LdotH)
{
    // Don't divide by Pi to avoid light being too dim.
    float f90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
    float lightScatter = F_Schlick(1.0, f90, NdotL);
    float viewScatter = F_Schlick(1.0, f90, NdotV);
    return lightScatter * viewScatter;
}

// Calculate distribution.
// Based on https://google.github.io/filament/Filament.html#citation-walter07
// D_GGX(h,α) = α^2 / { π ((n⋅h)^2 (α2−1) + 1)^2 }
// Simple implementation, has precision problems when using fp16 instead of fp32
// see https://google.github.io/filament/Filament.html#listing_speculardfp16
float D_GGX(float roughness, float NdotH)
{
    float oneMinusNdotHSquared = 1.0 - NdotH * NdotH;
    float a = NdotH * roughness;
    float k = roughness / (oneMinusNdotHSquared + a * a);
    return k * k * (1.0 / 3.14159265359);
}

// Calculate visibility.
// Hammon 2017, "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
// see https://google.github.io/filament/Filament.html#listing_approximatedspecularv
float V_Smith_GGX_Correlated(float roughness, float NdotV, float NdotL)
{
    return 0.5 / mix(2.0 * NdotL * NdotV, NdotL + NdotV, roughness);
}

// Fresnel function
// see https://google.github.io/filament/Filament.html#citation-schlick94
// F_Schlick(v,h,f_0,f_90) = f_0 + (f_90 − f_0) (1 − v⋅h)^5
vec3 fresnel(vec3 f0, float f90, float VdotH)
{
    return f0 + (f90 - f0) * pow(1.0 - VdotH, 5.0);
}

vec3 envBRDFApprox(vec3 F0, vec2 F_ab)
{
    return F0 * F_ab.x + F_ab.y;
}

float perceptualRoughnessToRoughness(float perceptual_roughness)
{
    float roughness = clamp(perceptual_roughness, 0.089, 1.0);
    return roughness * roughness;
}

vec3 environmentLight(
    vec3 irradiance,
    vec3 radiance,
    float roughness,
    vec3 diffuse_color,
    vec2 F_ab,
    vec3 F0,
    float F90,
    float NdotV)
{
    // Multiscattering approximation: https://www.jcgt.org/published/0008/01/03/paper.pdf
    // Useful reference: https://bruop.github.io/ibl
    vec3 Fr = max(vec3(1.0 - roughness), F0) - F0;
    vec3 kS = F0 + Fr * pow(1.0 - NdotV, 5.0);
    float Ess = F_ab.x + F_ab.y;
    vec3 FssEss = kS * Ess * F90;
    float Ems = 1.0 - Ess;
    vec3 Favg = F0 + (1.0 - F0) / 21.0;
    vec3 Fms = FssEss * Favg / (1.0 - Ems * Favg);
    vec3 FmsEms = Fms * Ems;
    vec3 Edss = 1.0 - (FssEss + FmsEms);
    vec3 kD = diffuse_color * Edss;

    vec3 diffuse = (FmsEms + kD) * irradiance;
    vec3 specular = FssEss * radiance;
    return diffuse + specular;
}
