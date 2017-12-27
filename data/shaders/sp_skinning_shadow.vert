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
layout(location = 12) in ivec2 i_misc_data_two;

#if defined(Use_Bindless_Texture)
layout(location = 13) in uvec4 i_bindless_texture_0;
layout(location = 14) in uvec4 i_bindless_texture_1;
layout(location = 15) in uvec4 i_bindless_texture_2;
#elif defined(Use_Array_Texture)
layout(location = 13) in uvec4 i_array_texture_0;
layout(location = 14) in uvec2 i_array_texture_1;
#endif

#stk_include "utils/get_world_location.vert"

out vec2 uv;

#if defined(Use_Bindless_Texture)
flat out sampler2D tex_layer_0;
flat out sampler2D tex_layer_1;
flat out sampler2D tex_layer_2;
flat out sampler2D tex_layer_3;
flat out sampler2D tex_layer_4;
flat out sampler2D tex_layer_5;
#elif defined(Use_Array_Texture)
flat out float array_0;
flat out float array_1;
flat out float array_2;
flat out float array_3;
flat out float array_4;
flat out float array_5;
#endif

void main()
{

#if defined(Converts_10bit_Vector)
    vec4 i_rotation = convert10BitVector(i_rotation_orig);
#endif

#if defined(Use_Bindless_Texture)
    tex_layer_0 = sampler2D(i_bindless_texture_0.xy);
    tex_layer_1 = sampler2D(i_bindless_texture_0.zw);
    tex_layer_2 = sampler2D(i_bindless_texture_1.xy);
    tex_layer_3 = sampler2D(i_bindless_texture_1.zw);
    tex_layer_4 = sampler2D(i_bindless_texture_2.xy);
    tex_layer_5 = sampler2D(i_bindless_texture_2.zw);
#elif defined(Use_Array_Texture)
    array_0 = float(i_array_texture_0.x);
    array_1 = float(i_array_texture_0.y);
    array_2 = float(i_array_texture_0.z);
    array_3 = float(i_array_texture_0.w);
    array_4 = float(i_array_texture_1.x);
    array_5 = float(i_array_texture_1.y);
#endif

#ifdef VSLayer
    gl_Layer = layer;
#endif

    vec4 idle_position = vec4(i_position, 1.0);
    vec4 skinned_position = vec4(0.0);
    int skinning_offset = i_misc_data_two.x;

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
