uniform int layer;

layout(location = 0) in vec3 i_position;
layout(location = 3) in vec2 i_uv;
layout(location = 8) in vec3 i_origin;
layout(location = 9) in vec4 i_rotation;
layout(location = 10) in vec4 i_scale;

#stk_include "utils/get_world_location.vert"

out vec2 uv;

void main()
{
    vec4 world_position = getWorldPosition(i_origin, i_rotation, i_scale.xyz,
        i_position);
    uv = i_uv;
    gl_Position = u_shadow_projection_view_matrices[layer] * world_position;
}
