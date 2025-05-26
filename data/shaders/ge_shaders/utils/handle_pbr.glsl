layout (set = 2, binding = 0) uniform samplerCube u_diffuse;
layout (set = 2, binding = 1) uniform samplerCube u_specular;

#include "camera.glsl"
#include "constants_utils.glsl"
#include "spm_data.glsl"
#include "pbr_utils.glsl"
#include "global_light_data.glsl"

#include "get_pos_from_frag_coord.glsl"
#include "pbr_light.glsl"
#include "sun_direction.glsl"

vec3 handlePBR(vec3 diffuse_color, vec3 pbr, vec3 normal)
{
    vec3 xpos = getPosFromFragCoord(gl_FragCoord, u_camera.m_viewport,
        u_camera.m_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos);
    vec3 reflection = reflect(-eyedir, normal);

    float perceptual_roughness = 1.0 - pbr.x;
    float radiance_level = perceptual_roughness * u_specular_levels_minus_one;
    vec3 world_normal = (u_camera.m_inverse_view_matrix * vec4(normal, 0.0)).xyz;
    vec3 world_reflection = (u_camera.m_inverse_view_matrix * vec4(reflection, 0.0)).xyz;

    vec3 irradiance = vec3(0.0);
    vec3 radiance = vec3(0.0);
    if (u_ibl)
    {
        irradiance = texture(u_diffuse, world_normal).rgb;
        radiance = textureLod(u_specular, world_reflection, radiance_level).rgb;
    }

    vec3 lightdir = sunDirection(reflection,
        u_global_light.m_sun_direction, u_global_light.m_sun_angle_tan_half,
        u_camera.m_inverse_view_matrix);

    vec3 mixed_color = PBRSunAmbientEmitLight(
        normal, eyedir, lightdir, diffuse_color,
        irradiance, radiance,
        u_global_light.m_sun_color,
        u_global_light.m_ambient_color,
        perceptual_roughness, pbr.y, pbr.z);

    mixed_color += accumulateLights(diffuse_color, normal, xpos, eyedir,
        perceptual_roughness, pbr.y);

    float factor = (1.0 - exp(length(xpos) * -0.0001));
    mixed_color = mixed_color + vec3(0.5) * factor;

    return convertColor(mixed_color);
}
