layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;
layout(location = 4) in float f_hue_change;
#ifdef PBR_ENABLED
layout(location = 5) in vec3 f_normal;
layout(location = 8) in vec4 f_world_position;
#endif

layout(location = 0) out vec4 o_color;

#include "utils/sample_mesh_texture.glsl"
#include "../utils/rgb_conversion.frag"
#ifdef PBR_ENABLED
#include "utils/handle_pbr.glsl"
#include "../utils/encode_normal.frag"
layout(location = 1) out vec4 o_normal;
#endif

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
    vec3 pbr = sampleMeshTexture2(f_material_id, f_uv).xyz;
    if (u_deferred)
    {
        o_color = vec4(diffuse_color, pbr.z);
        o_normal.xy = EncodeNormal(normal);
        o_normal.zw = pbr.xy;
    }
    else
    {
        o_color = vec4(handlePBR(diffuse_color, pbr, f_world_position, normal),
            1.0);
    }
#endif
}
