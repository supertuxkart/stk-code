uniform samplerBuffer skinning_tex;

layout(location = 0) in vec3 i_position;

#if defined(Converts_10bit_Vector)
layout(location = 1) in vec4 i_normal_orig;
#else
layout(location = 1) in vec4 i_normal;
#endif

#if defined(Converts_10bit_Vector)
layout(location = 5) in vec4 i_tangent_orig;
#else
layout(location = 5) in vec4 i_tangent;
#endif

layout(location = 6) in ivec4 i_joint;
layout(location = 7) in vec4 i_weight;
layout(location = 8) in vec3 i_origin;
layout(location = 9) in vec4 i_rotation;
layout(location = 10) in vec4 i_scale;
layout(location = 12) in ivec2 i_misc_data;

#stk_include "utils/get_world_location.vert"

out vec3 o_tangent;
out vec3 o_bitangent;
out vec3 o_normal;

void main()
{

#if defined(Converts_10bit_Vector)
    vec4 i_normal = convert10BitVector(i_normal_orig);
    vec4 i_tangent = convert10BitVector(i_tangent_orig);
#endif

    vec4 idle_position = vec4(i_position, 1.0);
    vec4 idle_normal = vec4(i_normal.xyz, 0.0);
    vec4 idle_tangent = vec4(i_tangent.xyz, 0.0);
    vec4 skinned_position = vec4(0.0);
    vec4 skinned_normal = vec4(0.0);
    vec4 skinned_tangent = vec4(0.0);
    int skinning_offset = i_misc_data.x;

#ifdef GL_ES
    mat4 joint_matrix =
        i_weight[0] * mat4(
        texelFetch(skinning_tex, ivec2(0, clamp(i_joint[0] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(1, clamp(i_joint[0] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(2, clamp(i_joint[0] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(3, clamp(i_joint[0] + skinning_offset, 0, MAX_BONES)), 0)) +
        i_weight[1] * mat4(
        texelFetch(skinning_tex, ivec2(0, clamp(i_joint[1] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(1, clamp(i_joint[1] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(2, clamp(i_joint[1] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(3, clamp(i_joint[1] + skinning_offset, 0, MAX_BONES)), 0)) +
        i_weight[2] * mat4(
        texelFetch(skinning_tex, ivec2(0, clamp(i_joint[2] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(1, clamp(i_joint[2] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(2, clamp(i_joint[2] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(3, clamp(i_joint[2] + skinning_offset, 0, MAX_BONES)), 0)) +
        i_weight[3] * mat4(
        texelFetch(skinning_tex, ivec2(0, clamp(i_joint[3] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(1, clamp(i_joint[3] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(2, clamp(i_joint[3] + skinning_offset, 0, MAX_BONES)), 0),
        texelFetch(skinning_tex, ivec2(3, clamp(i_joint[3] + skinning_offset, 0, MAX_BONES)), 0));
#else
    mat4 joint_matrix =
        i_weight[0] * mat4(
        texelFetch(skinning_tex, clamp(i_joint[0] + skinning_offset, 0, MAX_BONES) * 4),
        texelFetch(skinning_tex, clamp(i_joint[0] + skinning_offset, 0, MAX_BONES) * 4 + 1),
        texelFetch(skinning_tex, clamp(i_joint[0] + skinning_offset, 0, MAX_BONES) * 4 + 2),
        texelFetch(skinning_tex, clamp(i_joint[0] + skinning_offset, 0, MAX_BONES) * 4 + 3)) +
        i_weight[1] * mat4(
        texelFetch(skinning_tex, clamp(i_joint[1] + skinning_offset, 0, MAX_BONES) * 4),
        texelFetch(skinning_tex, clamp(i_joint[1] + skinning_offset, 0, MAX_BONES) * 4 + 1),
        texelFetch(skinning_tex, clamp(i_joint[1] + skinning_offset, 0, MAX_BONES) * 4 + 2),
        texelFetch(skinning_tex, clamp(i_joint[1] + skinning_offset, 0, MAX_BONES) * 4 + 3)) +
        i_weight[2] * mat4(
        texelFetch(skinning_tex, clamp(i_joint[2] + skinning_offset, 0, MAX_BONES) * 4),
        texelFetch(skinning_tex, clamp(i_joint[2] + skinning_offset, 0, MAX_BONES) * 4 + 1),
        texelFetch(skinning_tex, clamp(i_joint[2] + skinning_offset, 0, MAX_BONES) * 4 + 2),
        texelFetch(skinning_tex, clamp(i_joint[2] + skinning_offset, 0, MAX_BONES) * 4 + 3)) +
        i_weight[3] * mat4(
        texelFetch(skinning_tex, clamp(i_joint[3] + skinning_offset, 0, MAX_BONES) * 4),
        texelFetch(skinning_tex, clamp(i_joint[3] + skinning_offset, 0, MAX_BONES) * 4 + 1),
        texelFetch(skinning_tex, clamp(i_joint[3] + skinning_offset, 0, MAX_BONES) * 4 + 2),
        texelFetch(skinning_tex, clamp(i_joint[3] + skinning_offset, 0, MAX_BONES) * 4 + 3));
#endif

    skinned_position = joint_matrix * idle_position;
    skinned_normal = joint_matrix * idle_normal;
    skinned_tangent = joint_matrix * idle_tangent;

    float step_mix = step(float(skinning_offset), 0.0);
    skinned_position = mix(skinned_position, idle_position, step_mix);
    skinned_normal = mix(skinned_normal, idle_normal, step_mix);
    skinned_tangent = mix(skinned_tangent, idle_tangent, step_mix);

    gl_Position = getWorldPosition(i_origin, i_rotation, i_scale.xyz,
        skinned_position.xyz);
    o_normal = normalize(rotateVector(i_rotation, skinned_normal.xyz));
    o_tangent = normalize(rotateVector(i_rotation, skinned_tangent.xyz));
    o_bitangent = cross(o_normal, o_tangent) * i_tangent.w;

}
