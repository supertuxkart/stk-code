layout(location = 0) in vec3 i_position;

#if defined(Converts_10bit_Vector)
layout(location = 1) in vec4 i_normal_orig;
#else
layout(location = 1) in vec4 i_normal;
#endif

layout(location = 2) in vec4 i_color;
layout(location = 3) in vec2 i_uv;
layout(location = 4) in vec2 i_uv_two;

#if defined(Converts_10bit_Vector)
layout(location = 5) in vec4 i_tangent_orig;
#else
layout(location = 5) in vec4 i_tangent;
#endif

layout(location = 8) in vec3 i_origin;
layout(location = 9) in vec4 i_rotation;
layout(location = 10) in vec4 i_scale;
layout(location = 11) in vec2 i_texture_trans;
layout(location = 12) in ivec2 i_misc_data;

#stk_include "utils/get_world_location.vert"

out vec3 tangent;
out vec3 bitangent;
out vec3 normal;
out vec2 uv;
out vec2 uv_two;
out vec4 color;
out vec4 world_position;
out vec3 world_normal;
out float camdist;
out float hue_change;

void main()
{

#if defined(Converts_10bit_Vector)
    vec4 i_normal = convert10BitVector(i_normal_orig);
    vec4 i_tangent = convert10BitVector(i_tangent_orig);
#endif

    vec4 v_world_position = getWorldPosition(i_origin, i_rotation, i_scale.xyz,
        i_position);
    vec3 v_world_normal = rotateVector(i_rotation, i_normal.xyz);
    vec3 world_tangent = rotateVector(i_rotation, i_tangent.xyz);

    tangent = (u_view_matrix * vec4(world_tangent, 0.0)).xyz;
    bitangent = (u_view_matrix *
       // bitangent sign
      vec4(cross(v_world_normal, world_tangent) * i_tangent.w, 0.0)
      ).xyz;
    normal = (u_view_matrix * vec4(v_world_normal, 0.0)).xyz;

    uv = vec2(i_uv.x + (i_texture_trans.x * i_normal.w),
        i_uv.y + (i_texture_trans.y * i_normal.w));
    uv_two = i_uv_two;

    color = i_color.zyxw;
    camdist = length(u_view_matrix * v_world_position);
    hue_change = float(i_misc_data.y) * 0.01;
    gl_Position = u_projection_view_matrix * v_world_position;
    world_position = v_world_position;
    world_normal = v_world_normal;
}
