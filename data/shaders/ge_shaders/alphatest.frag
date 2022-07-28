layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;

layout(location = 0) out vec4 o_color;

#include "utils/sample_mesh_texture.h"

void main()
{
    vec4 mixed_color = sampleMeshTexture0(f_material_id, f_uv)* f_vertex_color;
    if (mixed_color.a < 0.5)
        discard;
    o_color = vec4(mixed_color.xyz, 1.0);
}
