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
    // Don't divide by Pi to avoid light being too dim.
    float oneMinusNdotHSquared = 1.0 - NdotH * NdotH;
    float a = NdotH * roughness;
    float k = roughness / (oneMinusNdotHSquared + a * a);
    return k * k;
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

vec3 PBRLight(
    vec3 normal,
    vec3 eyedir,
    vec3 lightdir,
    vec3 color,
    float perceptual_roughness,
    float metallic)
{
    float NdotV = max(dot(normal, eyedir), 0.0001);
    float NdotL = clamp(dot(normal, lightdir), 0.0, 1.0);

    vec2 F_ab = F_AB(perceptual_roughness, NdotV);

    vec3 H = normalize(eyedir + lightdir);
    float NdotH = clamp(dot(normal, H), 0.0, 1.0);
    float LdotH = clamp(dot(lightdir, H), 0.0, 1.0);

    vec3 diffuse_color = color * (1.0 - metallic);
    vec3 F0 = mix(vec3(0.04), color, metallic);
    // No real world material has specular values under 0.02, so we use this range as a
    // "pre-baked specular occlusion" that extinguishes the fresnel term, for artistic control.
    // See: https://google.github.io/filament/Filament.html#specularocclusion
    float F90 = clamp(dot(F0, vec3(50.0 * 0.33)), 0.0, 1.0);

    float roughness = perceptualRoughnessToRoughness(perceptual_roughness);

    vec3 diffuse = diffuse_color * Fd_Burley(roughness, NdotV, NdotL, NdotH);

    float D = D_GGX(roughness, NdotH);
    float V = V_Smith_GGX_Correlated(roughness, NdotV, NdotL);
    vec3 F = fresnel(F0, F90, LdotH);
    vec3 specular = D * V * F * (1.0 + F0 * (1.0 / F_ab.x - 1.0));

    return NdotL * (diffuse + specular);
}

vec3 PBRSunAmbientEmitLight(
    vec3 normal,
    vec3 eyedir,
    vec3 sundir,
    vec3 color,
    vec3 sun_color,
    vec3 ambient_color,
    float perceptual_roughness,
    float metallic,
    float emissive)
{
    // Copied from PBRLight to use F_ab and F90 again
    float NdotV = max(dot(normal, eyedir), 0.0001);
    float NdotL = clamp(dot(normal, sundir), 0.0, 1.0);

    vec2 F_ab = F_AB(perceptual_roughness, NdotV);

    vec3 H = normalize(eyedir + sundir);
    float NdotH = clamp(dot(normal, H), 0.0, 1.0);
    float LdotH = clamp(dot(sundir, H), 0.0, 1.0);

    vec3 diffuse_color = color * (1.0 - metallic);
    vec3 F0 = mix(vec3(0.04), color, metallic);
    // No real world material has specular values under 0.02, so we use this range as a
    // "pre-baked specular occlusion" that extinguishes the fresnel term, for artistic control.
    // See: https://google.github.io/filament/Filament.html#specularocclusion
    float F90 = clamp(dot(F0, vec3(50.0 * 0.33)), 0.0, 1.0);

    float roughness = perceptualRoughnessToRoughness(perceptual_roughness);

    vec3 diffuse = diffuse_color * Fd_Burley(roughness, NdotV, NdotL, NdotH);

    float D = D_GGX(roughness, NdotH);
    float V = V_Smith_GGX_Correlated(roughness, NdotV, NdotL);
    vec3 F = fresnel(F0, F90, LdotH);
    vec3 specular = D * V * F * (1.0 + F0 * (1.0 / F_ab.x - 1.0));

    vec3 sunlight = NdotL * (diffuse + specular);

    vec3 diffuse_ambient = envBRDFApprox(diffuse_color, F_AB(1.0, NdotV));

    vec3 specular_ambient = F90 * envBRDFApprox(F0, F_ab);

    vec3 emit = emissive * color * 4.0;

    return sun_color * sunlight + ambient_color * (diffuse_ambient + specular_ambient) + emit;
}
