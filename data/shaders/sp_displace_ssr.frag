in vec2 uv;
in vec3 normal;
in vec4 world_position;

layout(location = 0) out float o_displace_mask;
layout(location = 1) out vec4 o_displace_ssr;

#stk_include "utils/sp_texture_sampling.frag"
#stk_include "utils/screen_space_reflection.frag"

uniform samplerCube u_skybox_texture;
uniform sampler2D u_displace_color;
uniform sampler2DShadow u_depth;

uniform int u_ssr;

void main()
{
    o_displace_mask = 1.0;
    if (u_ssr == 0)
        return;

    float alpha = sampleTextureLayer0(uv).a;
    if (alpha == 0.0)
    {
        o_displace_ssr = vec4(0.0);
        return;
    }
    // eye-space position
    vec3 xpos = (u_view_matrix * world_position).xyz;
    // eye-space view direction (points from surface toward eye at origin)
    vec3 eyedir = -normalize(xpos);
    // eye-space normal
    vec3 normal = (u_view_matrix * vec4(normalize(normal), 0)).xyz;

    // bail out immediately if normal is facing away from the camera,
    // dot(normal, eyedir) <= 0 means back-facing
    float NdotV = dot(normal, eyedir);
    if (NdotV <= 0.0)
    {
        o_displace_ssr = vec4(0.0);
        return;
    }

    // compute reflection in eye-space
    vec3 reflected = reflect(-eyedir, normal);
    // bring it back into world-space
    vec3 world_reflection = (u_inverse_view_matrix * vec4(reflected, 0.0)).xyz;

    // fallback to skybox
    vec4 fallback = texture(u_skybox_texture, world_reflection);

    // early exit if normal is facing camera too directly (no meaningful reflection)
    if (normal.z < -0.75)
    {
        o_displace_ssr = fallback;
        return;
    }

    vec4 result;
    vec2 viewport_scale = vec2(1.0);
    vec2 viewport_offset = vec2(0.0);
    vec2 coords = RayCast(reflected, xpos, u_projection_matrix, viewport_scale,
        viewport_offset, u_depth);
    if (coords.x < 0. || coords.x > 1. || coords.y < 0. || coords.y > 1.)
    {
        result = fallback;
    }
    else
    {
        result = texture(u_displace_color, coords);
        float edge = GetEdgeFade(coords, viewport_scale, viewport_offset);
        //float fresnel = pow(1.0 - NdotV, 2.0);
        float fresnel = (1.0 - NdotV) * (1.0 - NdotV);
        float blend_weight = edge * fresnel;
        result = mix(fallback, result, blend_weight);
    }
    o_displace_ssr = result;
}
