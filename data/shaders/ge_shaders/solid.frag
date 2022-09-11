layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;
layout(location = 4) in float f_hue_change;

layout(location = 0) out vec4 o_color;

#include "utils/sample_mesh_texture.h"
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

    vec3 mixed_color = tex_color.xyz * f_vertex_color.xyz;
    o_color = vec4(mixed_color, 1.0);
}
