layout(set = 2, binding = 0) uniform samplerCube u_diffuse_environment_map;
layout(set = 2, binding = 1) uniform samplerCube u_specular_environment_map;
layout(set = 3, binding = 0) uniform sampler2DArrayShadow u_shadow_map;

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

vec3 environmentLight(
    vec3 world_normal,
    vec3 world_reflection,
    float perceptual_roughness,
    float roughness,
    vec3 diffuse_color,
    vec2 F_ab,
    vec3 F0,
    float F90,
    float NdotV)
{
    // Split-sum approximation for image based lighting: https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
    // Technically we could use textureNumLevels(specular_environment_map) - 1 here, but we use a uniform
    // because textureNumLevels() does not work on WebGL2
    float radiance_level = perceptual_roughness * 7.;

    float intensity = 0.8;

    vec3 irradiance = textureLod(
        u_diffuse_environment_map,
        world_normal, 0).rgb * intensity;

    vec3 radiance = textureLod(
        u_specular_environment_map,
        world_reflection, radiance_level).rgb * intensity;

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

float getShadowPCF(vec2 shadowtexcoord, float layer, float d)
{
    float shadow_res = 1024.;
    vec2 uv = shadowtexcoord * shadow_res;
    vec2 base_uv = floor(uv + 0.5);
    float s = (uv.x + 0.5 - base_uv.x);
    float t = (uv.y + 0.5 - base_uv.y);
    base_uv -= 0.5;
    base_uv /= shadow_res;

    float uw0 = (4.0 - 3.0 * s);
    float uw1 = 7.0;
    float uw2 = (1.0 + 3.0 * s);

    float u0 = (3.0 - 2.0 * s) / uw0 - 2.0;
    float u1 = (3.0 + s) / uw1;
    float u2 = s / uw2 + 2.0;

    float vw0 = (4.0 - 3.0 * t);
    float vw1 = 7.0;
    float vw2 = (1.0 + 3.0 * t);

    float v0 = (3.0 - 2.0 * t) / vw0 - 2.0;
    float v1 = (3.0 + t) / vw1;
    float v2 = t / vw2 + 2.0;

    float sum = 0.0;

    sum += uw0 * vw0 * texture(u_shadow_map, vec4(base_uv + (vec2(u0, v0) / shadow_res), layer, d));
    sum += uw1 * vw0 * texture(u_shadow_map, vec4(base_uv + (vec2(u1, v0) / shadow_res), layer, d));
    sum += uw2 * vw0 * texture(u_shadow_map, vec4(base_uv + (vec2(u2, v0) / shadow_res), layer, d));

    sum += uw0 * vw1 * texture(u_shadow_map, vec4(base_uv + (vec2(u0, v1) / shadow_res), layer, d));
    sum += uw1 * vw1 * texture(u_shadow_map, vec4(base_uv + (vec2(u1, v1) / shadow_res), layer, d));
    sum += uw2 * vw1 * texture(u_shadow_map, vec4(base_uv + (vec2(u2, v1) / shadow_res), layer, d));

    sum += uw0 * vw2 * texture(u_shadow_map, vec4(base_uv + (vec2(u0, v2) / shadow_res), layer, d));
    sum += uw1 * vw2 * texture(u_shadow_map, vec4(base_uv + (vec2(u1, v2) / shadow_res), layer, d));
    sum += uw2 * vw2 * texture(u_shadow_map, vec4(base_uv + (vec2(u2, v2) / shadow_res), layer, d));

    return sum / 144.0;
}

float getShadowFactor(vec3 world_position, vec3 xpos, vec3 normal, vec3 lightdir)
{
    float bias = max(1.0 - dot(normal, -lightdir), 0.1) / 1024.;
    float shadow = 1.0;
    
    if (xpos.z < 11.0)
    {
        vec4 light_view_position = u_camera.m_shadow_matrix_near * vec4(world_position.xyz, 1.0);
        light_view_position.xyz /= light_view_position.w;
        light_view_position.xy = light_view_position.xy * 0.5 + 0.5;
        light_view_position.z -= bias;
        shadow = getShadowPCF(light_view_position.xy, 0.0, light_view_position.z);
        if (xpos.z > 10.0)
        {
            light_view_position = u_camera.m_shadow_matrix_middle * vec4(world_position.xyz, 1.0);
            light_view_position.xyz /= light_view_position.w;
            light_view_position.xy = light_view_position.xy * 0.5 + 0.5;
            light_view_position.z -= bias;
            float factor = smoothstep(10.0, 11.0, xpos.z);
            shadow = mix(shadow, getShadowPCF(light_view_position.xy, 1.0, light_view_position.z), factor);
        }
    }
    else if (xpos.z < 40.0)
    {   
        vec4 light_view_position = u_camera.m_shadow_matrix_middle * vec4(world_position.xyz, 1.0);
        light_view_position.xyz /= light_view_position.w;
        light_view_position.xy = light_view_position.xy * 0.5 + 0.5;
        light_view_position.z -= bias;
        shadow = getShadowPCF(light_view_position.xy, 1.0, light_view_position.z);
        if (xpos.z > 35.0)
        {
            light_view_position = u_camera.m_shadow_matrix_far * vec4(world_position.xyz, 1.0);
            light_view_position.xyz /= light_view_position.w;
            light_view_position.xy = light_view_position.xy * 0.5 + 0.5;
            light_view_position.z -= bias;
            float factor = smoothstep(35.0, 40.0, xpos.z);
            shadow = mix(shadow, getShadowPCF(light_view_position.xy, 2.0, light_view_position.z), factor);
        }
    }
    else if (xpos.z < 150.0)
    {
        vec4 light_view_position = u_camera.m_shadow_matrix_far * vec4(world_position.xyz, 1.0);
        light_view_position.xyz /= light_view_position.w;
        light_view_position.xy = light_view_position.xy * 0.5 + 0.5;
        light_view_position.z -= bias;
        shadow = getShadowPCF(light_view_position.xy, 2.0, light_view_position.z);
        float factor = smoothstep(130.0, 150.0, xpos.z);
        shadow = (1.0 - factor) * shadow + factor * 1.0;
    }
    else shadow = 1.0;
    return shadow;
}

vec3 PBRSunAmbientEmitLight(
    vec3 normal,
    vec3 eyedir, 
    vec3 sundir,
    float shadow,
    vec3 world_normal,
    vec3 world_reflection,
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

    vec3 sunlight = NdotL * (diffuse + specular) * shadow;

    vec3 diffuse_ambient = envBRDFApprox(diffuse_color, F_AB(1.0, NdotV));

    vec3 specular_ambient = F90 * envBRDFApprox(F0, F_ab);

    vec3 environment = environmentLight(
        world_normal, world_reflection,
        perceptual_roughness, roughness,
        diffuse_color, F_ab, F0, F90, NdotV
    );

    vec3 emit = emissive * color * 4.0;

    return sun_color * sunlight
         + environment + emit;
}