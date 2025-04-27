layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;

layout(location = 0) out vec4 o_color;

#include "utils/constants_utils.glsl"
#include "utils/sample_mesh_texture.glsl"

void main()
{
    vec4 color = sampleMeshTexture0(f_material_id, f_uv) * f_vertex_color;
    vec3 mixed_color = color.xyz;
    float alpha = color.w;
#ifdef PBR_ENABLED
    mixed_color = convertColor(mixed_color);
#endif
    o_color = vec4(mixed_color * alpha, alpha);
}
