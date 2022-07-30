layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;

layout(location = 0) out vec4 o_color;

#include "utils/sample_mesh_texture.h"

void main()
{
    vec3 mixed_color = sampleMeshTexture0(f_material_id, f_uv).xyz * f_vertex_color.xyz;
    o_color = vec4(mixed_color * 0.5, 0.5);
}
