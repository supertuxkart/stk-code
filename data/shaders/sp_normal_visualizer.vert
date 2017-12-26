uniform samplerBuffer skinning_tex;

layout(location = 0) in vec3 i_position;

#if defined(Converts_10bit_Vector)
layout(location = 1) in int i_normal_pked;
#else
layout(location = 1) in vec4 i_normal;
#endif

layout(location = 6) in ivec4 i_joint;
layout(location = 7) in vec4 i_weight;
layout(location = 8) in vec3 i_origin;

#if defined(Converts_10bit_Vector)
layout(location = 9) in int i_rotation_pked;
#else
layout(location = 9) in vec4 i_rotation;
#endif

layout(location = 10) in vec4 i_scale;
layout(location = 12) in ivec2 i_misc_data_two;

#stk_include "utils/get_world_location.vert"

out vec2 uv;
out vec3 normal;

void main()
{

#if defined(Converts_10bit_Vector)
    vec4 i_normal = convert10BitVector(i_normal_pked);
    vec4 i_rotation = convert10BitVector(i_rotation_pked);
#endif

    vec4 idle_position = vec4(i_position, 1.0);
    vec4 idle_normal = vec4(i_normal.xyz, 0.0);
    vec4 skinned_position = vec4(0.0);
    vec4 skinned_normal = vec4(0.0);
    int skinning_offset = i_misc_data_two.x;

    for (int i = 0; i < 4; i++)
    {
        mat4 joint_matrix = mat4(
            texelFetch(skinning_tex,
                clamp(i_joint[i] + skinning_offset, 0, MAX_BONES) * 4),
            texelFetch(skinning_tex,
                clamp(i_joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 1),
            texelFetch(skinning_tex,
                clamp(i_joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 2),
            texelFetch(skinning_tex,
                clamp(i_joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 3));
        skinned_position += i_weight[i] * joint_matrix * idle_position;
        skinned_normal += i_weight[i] * joint_matrix * idle_normal;
    }

    float step_mix = step(float(skinning_offset), -32769.0);
    skinned_position = mix(idle_position, skinned_position, step_mix);
    skinned_normal = mix(idle_normal, skinned_normal, step_mix);

    vec4 world_position = getWorldPosition(i_origin, i_rotation, i_scale.xyz,
        skinned_position.xyz);
    vec3 world_normal = rotateVector(i_rotation, skinned_normal.xyz);
    normal = (u_view_matrix * vec4(world_normal, 0.0)).xyz;
    gl_Position = u_projection_view_matrix * world_position;
}
