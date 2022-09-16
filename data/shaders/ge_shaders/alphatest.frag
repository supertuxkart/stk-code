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
    if (tex_color.a * f_vertex_color.a < 0.5)
        discard;

    if (f_hue_change > 0.0)
    {
        vec3 old_hsv = rgbToHsv(tex_color.rgb);
        vec2 new_xy = vec2(f_hue_change, old_hsv.y);
        vec3 new_color = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
        tex_color = vec4(new_color.r, new_color.g, new_color.b, tex_color.a);
    }

    tex_color.xyz *= f_vertex_color.xyz;
    o_color = vec4(tex_color.xyz, 1.0);
}
