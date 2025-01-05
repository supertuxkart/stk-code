layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;
layout(location = 4) in float f_hue_change;
#ifdef PBR_ENABLED
layout(location = 5) in vec3 f_normal;
layout(location = 6) in vec3 f_tangent;
layout(location = 7) in vec3 f_bitangent;
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

    vec4 layer_3 = sampleMeshTexture3(f_material_id, f_uv);
    vec3 tangent_space_normal = 2.0 * layer_3.xyz - 1.0;
    vec3 frag_tangent = normalize(f_tangent);
    vec3 frag_bitangent = normalize(f_bitangent);
    vec3 frag_normal = normalize(f_normal);
    mat3 t_b_n = mat3(frag_tangent, frag_bitangent, frag_normal);

    vec3 normal = normalize(t_b_n * tangent_space_normal);

    vec4 pbr = sampleMeshTexture2(f_material_id, f_uv);
    vec3 xpos = getPosFromFragCoord(gl_FragCoord, u_camera.m_viewport, u_camera.m_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos);
    vec3 reflection = reflect(-eyedir, normal);

    vec3 lightdir = sunDirection(reflection,
                                u_global_light.m_sun_direction,
                                u_global_light.m_sun_angle_tan_half,
                                u_camera.m_inverse_view_matrix);

    vec4 world_position = u_camera.m_inverse_view_matrix * vec4(xpos.xyz, 1.0);
    float shadow = getShadowFactor(world_position.xyz, xpos, normal, lightdir);

    vec3 mixed_color = PBRSunAmbientEmitLight(
        normal, eyedir, lightdir, shadow,
        (u_camera.m_inverse_view_matrix * vec4(normal, 0.0)).xyz,
        (u_camera.m_inverse_view_matrix * vec4(reflection, 0.0)).xyz,
        diffuse_color,
        u_global_light.m_sun_color,
        u_global_light.m_ambient_color,
        1.0 - pbr.x, pbr.y, pbr.z);

    mixed_color = applyFog(
        eyedir, -lightdir, mixed_color,
        u_global_light.m_sun_color, u_global_light.m_sun_scatter * shadow,
        length(xpos), 
        u_global_light.m_fog_color, u_global_light.m_fog_density);

    mixed_color = (mixed_color * (6.9 * mixed_color + 0.5)) / (mixed_color * (5.2 * mixed_color + 1.7) + 0.06);

    o_color = vec4(mixed_color, 1.0);
#endif
}
