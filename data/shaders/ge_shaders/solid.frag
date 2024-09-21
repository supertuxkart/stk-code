layout(std140, set = 1, binding = 0) uniform CameraBuffer
{
    mat4 m_view_matrix;
    mat4 m_projection_matrix;
    mat4 m_inverse_view_matrix;
    mat4 m_inverse_projection_matrix;
    mat4 m_projection_view_matrix;
    mat4 m_inverse_projection_view_matrix;
    vec4 m_viewport;
} u_camera;

layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;
layout(location = 4) in float f_hue_change;
#ifdef PBR_ENABLED
layout(location = 5) in vec3 f_normal;
#endif

layout(location = 0) out vec4 o_color;

#include "utils/sample_mesh_texture.h"
#include "../utils/getPosFromUVDepth.frag"
#include "../utils/rgb_conversion.frag"

vec2 F_AB(float perceptual_roughness, float NdotV) 
{
    vec4 c0 = vec4(-1.0, -0.0275, -0.572, 0.022);
    vec4 c1 = vec4(1.0, 0.0425, 1.04, -0.04);
    vec4 r = perceptual_roughness * c0 + c1;
    float a004 = min(r.x * r.x, pow(2.0, -9.28 * NdotV)) * r.x + r.y;
    return vec2(-1.04, 1.04) * a004 + r.zw;
}

vec3 EnvBRDFApprox(vec3 F0, vec2 F_ab)
{
    return F0 * F_ab.x + F_ab.y;
}

// Lambert model
float F_Schlick(float f0, float f90, float VdotH)
{
    // not using mix to keep the vec3 and float versions identical
    return f0 + (f90 - f0) * pow(1.0 - VdotH, 5.0);
}

vec3 DiffuseBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness, float metallic)
{
    vec3 H = normalize(eyedir + lightdir);
    float NdotV = max(dot(normal, eyedir), 0.0001);
    float NdotL = clamp(dot(normal, lightdir), 0.0, 1.0);
    float LdotH = clamp(dot(lightdir, H), 0.0, 1.0);

    // clamp perceptual roughness to prevent precision problems
    // According to Filament design 0.089 is recommended for mobile
    // Filament uses 0.045 for non-mobile
    roughness = clamp(roughness, 0.089, 1.0);
    roughness = roughness * roughness;
 

    float f90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
    float lightScatter = F_Schlick(1.0, f90, NdotL);
    float viewScatter = F_Schlick(1.0, f90, NdotV);
    return color * lightScatter * viewScatter * (1.0 - metallic);
}

// Specular BRDF
// https://google.github.io/filament/Filament.html#materialsystem/specularbrdf
// Cook-Torrance approximation of the microfacet model integration using Fresnel law F to model f_m
// f_r(v,l) = { D(h,α) G(v,l,α) F(v,h,f0) } / { 4 (n⋅v) (n⋅l) }
vec3 SpecularBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness, float metallic)
{
    float NdotV = max(dot(normal, eyedir), 0.0001);
    vec3 F0 = mix(vec3(0.04), color, metallic);
    vec2 F_ab = F_AB(roughness, NdotV);
    vec3 H = normalize(eyedir + lightdir);
    float NdotL = clamp(dot(normal, lightdir), 0.0, 1.0);
    float NdotH = clamp(dot(normal, H), 0.0, 1.0);
    float LdotH = clamp(dot(lightdir, H), 0.0, 1.0);

    // clamp perceptual roughness to prevent precision problems
    // According to Filament design 0.089 is recommended for mobile
    // Filament uses 0.045 for non-mobile
    roughness = clamp(roughness, 0.089, 1.0);
    roughness = roughness * roughness;
 
    // Calculate distribution.
    // Based on https://google.github.io/filament/Filament.html#citation-walter07
    // D_GGX(h,α) = α^2 / { π ((n⋅h)^2 (α2−1) + 1)^2 }
    // Simple implementation, has precision problems when using fp16 instead of fp32
    // see https://google.github.io/filament/Filament.html#listing_speculardfp16
    float oneMinusNdotHSquared = 1.0 - NdotH * NdotH;
    float a = NdotH * roughness;
    float k = roughness / (oneMinusNdotHSquared + a * a);
    float D = k * k;

    // Calculate visibility.
    // V(v,l,a) = G(v,l,α) / { 4 (n⋅v) (n⋅l) }
    // such that f_r becomes
    // f_r(v,l) = D(h,α) V(v,l,α) F(v,h,f0)
    // where
    // V(v,l,α) = 0.5 / { n⋅l sqrt((n⋅v)^2 (1−α2) + α2) + n⋅v sqrt((n⋅l)^2 (1−α2) + α2) }
    // Note the two sqrt's, that may be slow on mobile, 
    // see https://google.github.io/filament/Filament.html#listing_approximatedspecularv
    float a2 = roughness * roughness;
    float lambdaV = NdotL * sqrt((NdotV - a2 * NdotV) * NdotV + a2);
    float lambdaL = NdotV * sqrt((NdotL - a2 * NdotL) * NdotL + a2);
    float V = 0.5 / (lambdaV + lambdaL);

    // Calculate the Fresnel term.
    // f_90 suitable for ambient occlusion
    // see https://google.github.io/filament/Filament.html#lighting/occlusion
    float f90 = clamp(dot(F0, vec3(50.0 * 0.33)), 0.0, 1.0);
    // Fresnel function
    // see https://google.github.io/filament/Filament.html#citation-schlick94
    // F_Schlick(v,h,f_0,f_90) = f_0 + (f_90 − f_0) (1 − v⋅h)^5
    vec3 F = F0 + (f90 - F0) * pow(1.0 - LdotH, 5.0);

    // Calculate the specular light.
    vec3 Fr = D * V * F * (1.0 + F0 * (1.0 / F_ab.x - 1.0));
    return Fr;
}

vec3 DiffuseIBL(vec3 normal, vec3 eyedir, vec3 color, float roughness, float metallic)
{
    float NdotV = max(dot(normal, eyedir), 0.0001);

    vec3 diffuse_ambient = EnvBRDFApprox(color * (1.0 - metallic), F_AB(1.0, NdotV));

    return diffuse_ambient;
}

vec3 SpecularIBL(vec3 normal, vec3 eyedir, vec3 color, float roughness, float metallic)
{
    float NdotV = max(dot(normal, eyedir), 0.0001);
    vec3 F0 = mix(vec3(0.04), color, metallic);

    vec3 specular_ambient = EnvBRDFApprox(F0, F_AB(roughness, NdotV));

    // No real world material has specular values under 0.02, so we use this range as a
    // "pre-baked specular occlusion" that extinguishes the fresnel term, for artistic control.
    // See: https://google.github.io/filament/Filament.html#specularocclusion
    float F90 = clamp(dot(F0, vec3(50.0 * 0.33)), 0.0, 1.0);

    return specular_ambient * F90;
}

vec3 SunMRP(vec3 normal, vec3 eyedir, vec3 sundirection)
{
    vec3 local_sundir = normalize((transpose(u_camera.m_inverse_view_matrix) * vec4(sundirection, 0.)).xyz);
    vec3 R = reflect(-eyedir, normal);
    float angularRadius = 3.14 * 100. / 180.;
    vec3 D = local_sundir;
    float d = cos(angularRadius);
    float r = sin(angularRadius);
    float DdotR = dot(D, R);
    vec3 S = R - DdotR * D;
    return (DdotR < d) ? normalize(d * D + normalize (S) * r) : R;
}

float LinearizeDepth(float depth)
{
    float near = 1.0;
    float far = 1500.0;
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * near) / (far + near - z * (far - near));
}

void main()
{
    vec4 tex_color = sampleMeshTexture0(f_material_id, f_uv);

    if (f_hue_change > 0.0)
    {
        float mask = tex_color.a;
        vec3 old_hsv = rgbToHsv(tex_color.rgb);
        float mask_step = step(mask, 0.5);
#ifndef PBR_ENABLED
        // For similar color
        float saturation = mask * 1.825; // 2.5 * 0.5 ^ (1. / 2.2)
#else
        float saturation = mask * 2.5;
#endif
        vec2 new_xy = mix(vec2(old_hsv.x, old_hsv.y), vec2(f_hue_change,
            max(old_hsv.y, saturation)), vec2(mask_step, mask_step));
        vec3 new_color = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
        tex_color = vec4(new_color.r, new_color.g, new_color.b, 1.0);
    }
#ifndef PBR_ENABLED
    vec3 mixed_color = tex_color.xyz * f_vertex_color.xyz;
    o_color = vec4(mixed_color, 1.0);
#else
    vec3 diffuse_color = tex_color.xyz * f_vertex_color.xyz;
    vec3 normal = normalize(f_normal.xyz);

    vec4 pbr = sampleMeshTexture2(f_material_id, f_uv);
    vec4 xpos4 = u_camera.m_inverse_projection_matrix *
                 vec4(gl_FragCoord.x / u_camera.m_viewport.z * 2.0 - 1.0,
                      gl_FragCoord.y / u_camera.m_viewport.w * 2.0 - 1.0,
                      gl_FragCoord.z, 1.0) / gl_FragCoord.w;
    vec3 xpos = xpos4.xyz;
    float roughness = 1.0 - pbr.x;
    float metallic = pbr.y;
    vec3 eyedir = -normalize(xpos);

    vec4 sun = u_camera.m_view_matrix * vec4(-2129.88, 487.03, 23.66, 0.0);
    vec3 Lightdir = normalize(sun.xyz);
    float NdotL = clamp(dot(normal, Lightdir), 0., 1.);

    vec3 Specular = NdotL * SpecularBRDF(normal, eyedir, Lightdir, diffuse_color, roughness, metallic) * vec3(0.5)
                          + SpecularIBL(normal, eyedir, diffuse_color, roughness, metallic) * vec3(0.3);
    vec3 Diffuse = NdotL * DiffuseBRDF(normal, eyedir, Lightdir, diffuse_color, roughness, metallic) * vec3(0.5)
                         + DiffuseIBL(normal, eyedir, diffuse_color, roughness, metallic) * vec3(0.3);

    vec3 mixed_color = Specular + Diffuse;

    float factor = (1.0 - exp(length(xpos) * -0.00001));
    mixed_color = mixed_color + vec3(0.5) * factor;

    mixed_color = (mixed_color * (6.9 * mixed_color + 0.5)) / (mixed_color * (5.2 * mixed_color + 1.7) + 0.06);

    o_color = vec4(mixed_color, 1.0);
#endif
}
