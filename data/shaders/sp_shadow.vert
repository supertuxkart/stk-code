uniform int layer;

layout(location = 0) in vec3 i_position;
layout(location = 3) in vec2 i_uv;
layout(location = 8) in vec3 i_origin;

#if defined(Converts_10bit_Vector)
layout(location = 9) in vec4 i_rotation_orig;
#else
layout(location = 9) in vec4 i_rotation;
#endif

layout(location = 10) in vec4 i_scale;

#stk_include "utils/get_world_location.vert"

out vec2 uv;

void main()
{

#if defined(Converts_10bit_Vector)
    vec4 i_rotation = convert10BitVector(i_rotation_orig);
#endif

    vec4 quaternion = normalize(vec4(i_rotation.xyz, i_scale.w));
    vec4 world_position = getWorldPosition(i_origin, quaternion, i_scale.xyz,
        i_position);
    uv = i_uv;
    gl_Position = u_shadow_projection_view_matrices[layer] * world_position;
}
