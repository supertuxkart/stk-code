layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;

layout(location = 0) out vec4 o_color;

layout(push_constant) uniform Constants
{
    vec4 m_displace_direction;
} u_push_constants;

#include "utils/camera.glsl"
#include "utils/constants_utils.glsl"
#include "utils/sample_mesh_texture.glsl"
#include "../utils/displace_utils.frag"

layout (set = 3, binding = 0) uniform sampler2D u_displace_mask;
layout (set = 3, binding = 1) uniform sampler2D u_displace_ssr;

void main()
{
#ifdef PBR_ENABLED
    vec4 color = sampleMeshTexture0(f_material_id, f_uv) * f_vertex_color;
    vec3 mixed_color = color.xyz;
    float alpha = color.w;
    mixed_color = convertColor(mixed_color);
    if (u_ssr)
    {
        float alpha = sampleMeshTexture0(f_material_id, f_uv).a;
        if (alpha == 0.0)
        {
            o_color = vec4(mixed_color * alpha, alpha);
            return;
        }
        float horiz = sampleMeshTexture2(f_material_id, f_uv + u_push_constants.m_displace_direction.xy * 150.).x;
        float vert = sampleMeshTexture2(f_material_id, (f_uv.yx + u_push_constants.m_displace_direction.zw * 150.) * vec2(0.9)).x;
        vec2 shift = getDisplaceShift(horiz, vert);
        ivec2 uv = getDisplaceUV(shift, u_camera.m_viewport, u_displace_mask);
        vec3 reflection = texelFetch(u_displace_ssr, uv, 0).xyz;
        o_color = vec4(mixed_color * alpha * 0.5 + reflection * alpha * 0.5 ,
            alpha);
    }
    else
    {
        o_color = vec4(mixed_color * alpha, alpha);
    }
#endif
}
