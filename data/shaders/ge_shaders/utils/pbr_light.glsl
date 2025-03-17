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
    float metallic,
    float specular_intensity)
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

    return NdotL * (diffuse + specular * specular_intensity);
}

float getDistanceAttenuation(float distanceSquare, float inverseRangeSquared)
{
    float factor = distanceSquare * inverseRangeSquared;
    float smoothFactor = clamp(1.0 - factor * factor, 0.0, 1.0);
    float attenuation = smoothFactor * smoothFactor;
    return attenuation * 1.0 / max(distanceSquare, 0.0001);
}

vec3 environmentLight(
    samplerCube diffuse_map,
    samplerCube specular_map,
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

    float intensity = 1.0;

    vec3 irradiance = textureLod(diffuse_map, world_normal, 0).rgb * intensity;

    vec3 radiance = textureLod(specular_map, world_reflection, radiance_level).rgb * intensity;

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

float getShadowPCF(sampler2DArrayShadow map, vec2 shadowtexcoord, int layer, float depth, float size)
{
    // Castaño, 2013, "Shadow Mapping Summary Part 1"
    vec2 uv = shadowtexcoord * size + 0.5;
    vec2 base = (floor(uv) - 0.5) / size;
    vec2 st = fract(uv);

    vec2 uw = vec2(3.0 - 2.0 * st.x, 1.0 + 2.0 * st.x);
    vec2 vw = vec2(3.0 - 2.0 * st.y, 1.0 + 2.0 * st.y);

    vec2 u = vec2((2.0 - st.x) / uw.x - 1.0, st.x / uw.y + 1.0) / size;
    vec2 v = vec2((2.0 - st.y) / vw.x - 1.0, st.y / vw.y + 1.0) / size;

    float sum = 0.0;
    sum += uw.x * vw.x * texture(map, vec4(base + vec2(u.x, v.x), float(layer), depth));
    sum += uw.y * vw.x * texture(map, vec4(base + vec2(u.y, v.x), float(layer), depth));
    sum += uw.x * vw.y * texture(map, vec4(base + vec2(u.x, v.y), float(layer), depth));
    sum += uw.y * vw.y * texture(map, vec4(base + vec2(u.y, v.y), float(layer), depth));
    return sum * (1.0 / 16.0);
}

vec3 getNormalBias(vec3 normal, vec3 lightdir, float texel)
{
	vec3 normal_bias = normal * texel;
	normal_bias -= lightdir * dot(lightdir, normal_bias);
    return normal_bias;
}

float getShadowFactor(sampler2DArrayShadow map, vec3 world_position, float view_depth, float NdotL, vec3 normal, vec3 lightdir)
{
    float end_factor = smoothstep(130., 150., view_depth);
    if (view_depth >= 150. || NdotL <= 0.001)
    {
        return end_factor;
    }

    float shadow = 1.0;
    float size = 1024.;
    float factor = smoothstep(9.0, 10.0, view_depth) + smoothstep(40.0, 45.0, view_depth);
    int level = int(factor);

    vec2 base_normal_bias = (u_camera.m_shadow_view_matrix * vec4(normal, 0.)).xy;
    base_normal_bias *= (1.0 - max(0.0, dot(-normal, lightdir))) / size;

    vec4 light_view_position = u_camera.m_shadow_matrix[level] * vec4(world_position, 1.0);
    light_view_position.xyz /= light_view_position.w;
    light_view_position.xy = light_view_position.xy * 0.5 + 0.5 + base_normal_bias;

    shadow = mix(getShadowPCF(map, light_view_position.xy, level, light_view_position.z, size), 1.0, end_factor);

    if (factor == float(level))
    {
        return shadow;
    }

    // Blend with next cascade by factor
    light_view_position = u_camera.m_shadow_matrix[level + 1] * vec4(world_position, 1.0);
    light_view_position.xyz /= light_view_position.w;
    light_view_position.xy = light_view_position.xy * 0.5 + 0.5 + base_normal_bias;

    shadow = mix(shadow, getShadowPCF(map, light_view_position.xy, level + 1, light_view_position.z, size), factor - float(level));

    return shadow;
}

vec3 PBRSunAmbientEmitLight(
    vec3 normal,
    vec3 eyedir, 
    vec3 sundir,
    float shadow,
    float NdotL,
    samplerCube diffuse_map,
    samplerCube specular_map,
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
        diffuse_map, specular_map,
        world_normal, world_reflection,
        perceptual_roughness, roughness,
        diffuse_color, F_ab, F0, F90, NdotV
    );

    vec3 emit = emissive * (color + emissive * emissive * color * color * 10.0);

    return sun_color * sunlight
         + environment + emit
         + (diffuse_ambient + specular_ambient) * ambient_color;
}
