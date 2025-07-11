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
    vec3 irradiance,
    vec3 radiance,
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

    // Other 0.6 comes from skybox
    ambient_color *= 0.4;
    vec3 environment;
    if (u_ibl)
    {
        environment = environmentLight(irradiance, radiance, roughness,
            diffuse_color, F_ab, F0, F90, NdotV);
    }
    else
    {
        environment = u_global_light.m_skytop_color * ambient_color *
            diffuse_color;
    }

    vec3 emit = emissive * color * 4.0;

    return sun_color * sunlight
          + environment + emit
          + (diffuse_ambient + specular_ambient) * ambient_color;
}

vec3 accumulateLights(int light_count, vec3 diffuse_color, vec3 normal,
                      vec3 xpos, vec3 eyedir, float perceptual_roughness,
                      float metallic)
{
    vec3 accumulated_color = vec3(0.0);
    for (int i = 0; i < light_count; i++)
    {
        vec3 light_to_frag = (u_camera.m_view_matrix *
            vec4(u_global_light.m_lights[i].m_position_radius.xyz,
            1.0)).xyz - xpos;
        float invrange = u_global_light.m_lights[i].m_color_inverse_square_range.w;
        float distance_sq = dot(light_to_frag, light_to_frag);
        if (distance_sq * invrange > 1.)
            continue;
        // SpotLight
        float sattenuation = 1.;
        float sscale = u_global_light.m_lights[i].m_direction_scale_offset.z;
        float distance = sqrt(distance_sq);
        float distance_inverse = 1. / distance;
        vec3 L = light_to_frag * distance_inverse;
        if (sscale != 0.)
        {
            vec3 sdir =
                vec3(u_global_light.m_lights[i].m_direction_scale_offset.xy, 0.);
            sdir.z = sqrt(1. - dot(sdir, sdir)) * sign(sscale);
            sdir = (u_camera.m_view_matrix * vec4(sdir, 0.0)).xyz;
            sattenuation = clamp(dot(-sdir, L) *
                abs(sscale) +
                u_global_light.m_lights[i].m_direction_scale_offset.w, 0.0, 1.0);
#ifndef TILED_GPU
            // Reduce branching in tiled GPU
            if (sattenuation == 0.)
                continue;
#endif
        }
        vec3 diffuse_specular = PBRLight(normal, eyedir, L, diffuse_color,
            perceptual_roughness, metallic);
        float attenuation = 20. / (1. + distance_sq);
        float radius = u_global_light.m_lights[i].m_position_radius.w;
        attenuation *= (radius - distance) / radius;
        attenuation *= sattenuation * sattenuation;
        vec3 light_color =
            u_global_light.m_lights[i].m_color_inverse_square_range.xyz;
        accumulated_color += light_color * attenuation * diffuse_specular;
    }
    return accumulated_color;
}

// Copied because reusing in a loop will be slower
vec3 calculateLight(int i, vec3 diffuse_color, vec3 normal, vec3 xpos,
                    vec3 eyedir, float perceptual_roughness, float metallic)
{
    vec3 light_to_frag = (u_camera.m_view_matrix *
        vec4(u_global_light.m_lights[i].m_position_radius.xyz,
        1.0)).xyz - xpos;
    float invrange = u_global_light.m_lights[i].m_color_inverse_square_range.w;
    float distance_sq = dot(light_to_frag, light_to_frag);
    if (distance_sq * invrange > 1.)
        return vec3(0.0);
    // SpotLight
    float sattenuation = 1.;
    float sscale = u_global_light.m_lights[i].m_direction_scale_offset.z;
    float distance = sqrt(distance_sq);
    float distance_inverse = 1. / distance;
    vec3 L = light_to_frag * distance_inverse;
    if (sscale != 0.)
    {
        vec3 sdir =
            vec3(u_global_light.m_lights[i].m_direction_scale_offset.xy, 0.);
        sdir.z = sqrt(1. - dot(sdir, sdir)) * sign(sscale);
        sdir = (u_camera.m_view_matrix * vec4(sdir, 0.0)).xyz;
        sattenuation = clamp(dot(-sdir, L) *
            abs(sscale) +
            u_global_light.m_lights[i].m_direction_scale_offset.w, 0.0, 1.0);
        if (sattenuation == 0.)
            return vec3(0.0);
    }
    vec3 diffuse_specular = PBRLight(normal, eyedir, L, diffuse_color,
        perceptual_roughness, metallic);
    float attenuation = 20. / (1. + distance_sq);
    float radius = u_global_light.m_lights[i].m_position_radius.w;
    attenuation *= (radius - distance) / radius;
    attenuation *= sattenuation * sattenuation;
    vec3 light_color =
        u_global_light.m_lights[i].m_color_inverse_square_range.xyz;
    return light_color * attenuation * diffuse_specular;
}
