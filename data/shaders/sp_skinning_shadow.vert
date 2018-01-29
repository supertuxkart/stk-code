uniform int layer;

#ifdef GL_ES
uniform sampler2D skinning_tex;
#else
uniform samplerBuffer skinning_tex;
#endif

layout(location = 0) in vec3 i_position;
layout(location = 3) in vec2 i_uv;
layout(location = 6) in ivec4 i_joint;
layout(location = 7) in vec4 i_weight;
layout(location = 8) in vec3 i_origin;

#if defined(Converts_10bit_Vector)
layout(location = 9) in vec4 i_rotation_orig;
#else
layout(location = 9) in vec4 i_rotation;
#endif

layout(location = 10) in vec4 i_scale;
layout(location = 12) in ivec2 i_misc_data;

#stk_include "utils/get_world_location.vert"

out vec2 uv;

void main()
{

#if defined(Converts_10bit_Vector)
    vec4 i_rotation = convert10BitVector(i_rotation_orig);
#endif

    vec4 idle_position = vec4(i_position, 1.0);
    vec4 skinned_position = vec4(0.0);
    int skinning_offset = i_misc_data.x;

    for (int i = 0; i < 4; i++)
    {
#ifdef GL_ES
        mat4 joint_matrix = mat4(
            texelFetch(skinning_tex, ivec2
                (0, clamp(i_joint[i] + skinning_offset, 0, MAX_BONES)), 0),
            texelFetch(skinning_tex, ivec2
                (1, clamp(i_joint[i] + skinning_offset, 0, MAX_BONES)), 0),
            texelFetch(skinning_tex, ivec2
                (2, clamp(i_joint[i] + skinning_offset, 0, MAX_BONES)), 0),
            texelFetch(skinning_tex, ivec2
                (3, clamp(i_joint[i] + skinning_offset, 0, MAX_BONES)), 0));
#else
        mat4 joint_matrix = mat4(
            texelFetch(skinning_tex,
                clamp(i_joint[i] + skinning_offset, 0, MAX_BONES) * 4),
            texelFetch(skinning_tex,
                clamp(i_joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 1),
            texelFetch(skinning_tex,
                clamp(i_joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 2),
            texelFetch(skinning_tex,
                clamp(i_joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 3));
#endif
        skinned_position += i_weight[i] * joint_matrix * idle_position;
    }

    vec4 quaternion = normalize(vec4(i_rotation.xyz, i_scale.w));
    vec4 world_position = getWorldPosition(i_origin, quaternion, i_scale.xyz,
        skinned_position.xyz);
    uv = i_uv;
    gl_Position = u_shadow_projection_view_matrices[layer] * world_position;
}
