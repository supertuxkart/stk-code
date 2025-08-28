layout(location = 1) in vec2 f_uv;
layout(location = 2) in vec2 f_uv_two;
layout(location = 3) flat in int f_material_id;
#ifdef PBR_ENABLED
layout(location = 5) in vec3 f_normal;
layout(location = 8) in vec4 f_world_position;
#endif

layout(location = 0) out vec4 o_color;

#include "utils/sample_mesh_texture.glsl"
#ifdef PBR_ENABLED
#include "utils/handle_pbr.glsl"
#include "../utils/encode_normal.frag"
layout(location = 1) out vec4 o_normal;
#endif

void main()
{
    vec4 color = sampleMeshTexture0(f_material_id, f_uv);
    vec4 layer_two_tex = sampleMeshTexture1(f_material_id, f_uv_two);
    layer_two_tex.rgb = layer_two_tex.a * layer_two_tex.rgb;
    vec3 final_color = layer_two_tex.rgb + color.rgb * (1.0 - layer_two_tex.a);
#ifndef PBR_ENABLED
    o_color = vec4(final_color, 1.0);
#else
    vec3 normal = normalize(f_normal.xyz);
    vec3 pbr = sampleMeshTexture2(f_material_id, f_uv).xyz;
    if (u_deferred)
    {
        o_color = vec4(final_color, pbr.z);
        o_normal.xy = EncodeNormal(normal);
        o_normal.zw = pbr.xy;
    }
    else
    {
        o_color = vec4(handlePBR(final_color, pbr, f_world_position, normal),
            1.0);
    }
#endif
}
