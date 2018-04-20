uniform int layer;
uniform vec3 wind_direction;

layout(location = 0) in vec3 i_position;
layout(location = 2) in vec4 i_color;
layout(location = 3) in vec2 i_uv;
layout(location = 8) in vec3 i_origin;
layout(location = 9) in vec4 i_rotation;
layout(location = 10) in vec4 i_scale;

#stk_include "utils/get_world_location.vert"

out vec2 uv;

void main()
{
    vec3 test = sin(wind_direction * (i_position.y * 0.1));
    test += cos(wind_direction) * 0.7;
    vec4 world_position = getWorldPosition(i_origin + test * i_color.r,
        i_rotation, i_scale.xyz, i_position);

    uv = i_uv;
    gl_Position = u_shadow_projection_view_matrices[layer] * world_position;
}
