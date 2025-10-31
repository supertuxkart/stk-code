layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;

#include "utils/sample_mesh_texture.glsl"

void main()
{
    vec4 tex_color = sampleMeshTexture0(f_material_id, f_uv);
    if (tex_color.a * f_vertex_color.a < 0.5)
        discard;
}
