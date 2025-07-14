layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;

layout(location = 0) out vec2 o_displace_mask;

layout(push_constant) uniform Constants
{
    vec4 m_displace_direction;
} u_push_constants;

#include "utils/sample_mesh_texture.glsl"

void main()
{
#ifdef PBR_ENABLED
    float horiz = sampleMeshTexture2(f_material_id, f_uv + u_push_constants.m_displace_direction.xy * 150.).x;
    float vert = sampleMeshTexture2(f_material_id, (f_uv.yx + u_push_constants.m_displace_direction.zw * 150.) * vec2(0.9)).x;

    vec2 offset = vec2(horiz, vert);
    offset = 2.0 * offset - 1.0;

    vec4 shiftval;
    shiftval.r = step(offset.x, 0.0) * -offset.x;
    shiftval.g = step(0.0, offset.x) * offset.x;
    shiftval.b = step(offset.y, 0.0) * -offset.y;
    shiftval.a = step(0.0, offset.y) * offset.y;

    vec2 mask;
    mask.x = -shiftval.x + shiftval.y;
    mask.y = -shiftval.z + shiftval.w;
    mask = (mask + 1.0) * 0.5;
    o_displace_mask = mask;
#endif
}
