layout(binding = 0) uniform sampler2D u_displace_mask;
layout(binding = 1) uniform sampler2D u_displace_color;

layout(location = 0) in vec2 f_uv;

layout(location = 0) out vec4 o_color;

layout(push_constant) uniform Constants
{
    bool m_has_displace;
} u_push_constants;

#include "utils/camera.glsl"

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
            shift *= 0.02 * u_camera.m_viewport.zw;
            ivec2 lo = ivec2(u_camera.m_viewport.xy);
            ivec2 hi = ivec2(u_camera.m_viewport.xy + u_camera.m_viewport.zw);
            ivec2 suv = clamp(ivec2(gl_FragCoord.xy) + ivec2(shift), lo, hi);
            vec2 new_mask = texelFetch(u_displace_mask, suv, 0).xy;
            if (!(new_mask.x == 0.0 && new_mask.y == 0.0))
                uv = suv;
        }
    }
    o_color = texelFetch(u_displace_color, uv, 0);
#endif
}
