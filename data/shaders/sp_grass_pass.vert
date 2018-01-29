uniform vec3 wind_direction;

layout(location = 0) in vec3 i_position;

#if defined(Converts_10bit_Vector)
layout(location = 1) in vec4 i_normal_orig;
#else
layout(location = 1) in vec4 i_normal;
#endif

layout(location = 2) in vec4 i_color;
layout(location = 3) in vec2 i_uv;
layout(location = 8) in vec3 i_origin;

#if defined(Converts_10bit_Vector)
layout(location = 9) in vec4 i_rotation_orig;
#else
layout(location = 9) in vec4 i_rotation;
#endif

layout(location = 10) in vec4 i_scale;
layout(location = 12) in ivec2 i_misc_data;

#stk_include "utils/get_world_location.vert"

out vec3 normal;
out vec2 uv;
flat out float hue_change;

void main()
{

#if defined(Converts_10bit_Vector)
    vec4 i_normal = convert10BitVector(i_normal_orig);
    vec4 i_rotation = convert10BitVector(i_rotation_orig);
#endif

    vec3 test = sin(wind_direction * (i_position.y * 0.1));
    test += cos(wind_direction) * 0.7;

    vec4 quaternion = normalize(vec4(i_rotation.xyz, i_scale.w));
    vec4 world_position = getWorldPosition(i_origin + test * i_color.r,
        quaternion, i_scale.xyz, i_position);
    vec3 world_normal = rotateVector(quaternion, i_normal.xyz);

    normal = (u_view_matrix * vec4(world_normal, 0.0)).xyz;
    uv = i_uv;
    hue_change = float(i_misc_data.y) * 0.01;
    gl_Position = u_projection_view_matrix * world_position;
}
