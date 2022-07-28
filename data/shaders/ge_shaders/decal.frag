layout(location = 1) in vec2 f_uv;
layout(location = 2) in vec2 f_uv_two;
layout(location = 3) flat in int f_material_id;

layout(location = 0) out vec4 o_color;

#include "utils/sample_mesh_texture.h"

void main()
{
    vec4 color = sampleMeshTexture0(f_material_id, f_uv);
    vec4 layer_two_tex = sampleMeshTexture1(f_material_id, f_uv_two);
    layer_two_tex.rgb = layer_two_tex.a * layer_two_tex.rgb;
    vec3 final_color = layer_two_tex.rgb + color.rgb * (1.0 - layer_two_tex.a);
    o_color = vec4(final_color, 1.0);
}
