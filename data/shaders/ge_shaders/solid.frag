layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;
layout(location = 4) in float f_hue_change;
#ifdef PBR_ENABLED
layout(location = 5) in vec3 f_normal;
#endif

layout(location = 0) out vec4 o_color;

#include "utils/camera.glsl"
#include "utils/fog.glsl"
#include "utils/get_pos_from_frag_coord.glsl"
#include "utils/global_light_data.glsl"
#include "utils/pbr_light.glsl"
#include "utils/sample_mesh_texture.glsl"
#include "utils/sun_direction.glsl"
#include "../utils/rgb_conversion.frag"

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
    vec3 xpos = getPosFromFragCoord(gl_FragCoord, u_camera.m_viewport, u_camera.m_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos);
    vec3 reflection = reflect(-eyedir, normal);

    vec3 lightdir = sunDirection(reflection,
                                u_camera.m_sun_direction,
                                u_camera.m_sun_angle_tan_half);

    vec4 world_position = u_camera.m_inverse_view_matrix * vec4(xpos.xyz, 1.0);
    vec4 light_view_position = u_camera.m_light_view_matrix * vec4(world_position.xyz, 1.0);
    light_view_position.xyz /= light_view_position.w;
    light_view_position.xy = light_view_position.xy * 0.5 + 0.5;
    float shadow = xpos.z > 150.0 ? 1.0 : texture(u_shadow_map, light_view_position.xyz - vec3(0., 0., max(dot(normal, lightdir), 0.) / 2048.));

    vec3 mixed_color = PBRSunAmbientEmitLight(
        normal, eyedir, lightdir, shadow,
        (u_camera.m_inverse_view_matrix * vec4(normal, 0.0)).xyz,
        (u_camera.m_inverse_view_matrix * vec4(reflection, 0.0)).xyz,
        diffuse_color,
        u_camera.m_sun_color,
        u_camera.m_ambient_color,
        1.0 - pbr.x, pbr.y, pbr.z);

    mixed_color = applyFog(
        eyedir, -lightdir, mixed_color,
        u_camera.m_sun_color, u_camera.m_sun_scatter * shadow,
        length(xpos), 
        u_camera.m_fog_color, u_camera.m_fog_density);

    mixed_color = (mixed_color * (6.9 * mixed_color + 0.5)) / (mixed_color * (5.2 * mixed_color + 1.7) + 0.06);

    o_color = vec4(mixed_color, 1.0);
#endif
}
