layout(binding = 0) uniform sampler2D u_displace_mask;
layout(binding = 2) uniform sampler2D u_displace_color;

layout(location = 0) in vec2 f_uv;

layout(location = 0) out vec4 o_color;

layout(push_constant) uniform Constants
{
    bool m_has_displace;
} u_push_constants;

#include "utils/camera.glsl"
#include "../utils/displace_utils.frag"

void main()
{
#ifdef PBR_ENABLED
    ivec2 uv = ivec2(gl_FragCoord.xy);
    if (u_push_constants.m_has_displace)
    {
        vec2 mask = texelFetch(u_displace_mask, uv, 0).xy;
        if (!(mask.x == 0.0 && mask.y == 0.0))
        {
            vec2 shift = 2.0 * mask - 1.0;
            uv = getDisplaceUV(shift, u_camera.m_viewport, u_displace_mask);
        }
    }
    o_color = texelFetch(u_displace_color, uv, 0);
#endif
}
