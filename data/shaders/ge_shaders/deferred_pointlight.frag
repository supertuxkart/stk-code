layout (input_attachment_index = 0, binding = 0) uniform subpassInput u_color;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput u_normal;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput u_depth;

layout(location = 0) flat in int light_idx;

layout(location = 0) out vec4 o_color;

#include "utils/unproject_position.glsl"
#include "utils/handle_pbr.glsl"
#include "../utils/decodeNormal.frag"

void main()
{
    float depth = subpassLoad(u_depth).x;
    if (depth == 1.0)
    {
        o_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    vec3 diffuse_color = subpassLoad(u_color).xyz;
    vec3 pbr = vec3(subpassLoad(u_normal).zw, subpassLoad(u_color).w);
    vec3 world_normal = DecodeNormal(subpassLoad(u_normal).xy);
    vec3 xpos = getPosFromUVDepth(vec3(gl_FragCoord.xy, depth),
        u_camera.m_viewport, u_camera.m_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos);
    vec3 normal = (u_camera.m_view_matrix * vec4(world_normal, 0.0)).xyz;
    vec3 light = calculateLight(light_idx, diffuse_color, normal, xpos,
        eyedir, 1.0 - pbr.x, pbr.y);
    o_color = vec4(light, 1.0);
}
