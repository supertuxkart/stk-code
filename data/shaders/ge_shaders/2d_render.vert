layout(location = 0) in vec2 v_position;
layout(location = 1) in vec4 v_color;
layout(location = 2) in vec2 v_uv;
layout(location = 3) in int v_sampler_index;

layout(location = 0) out vec4 f_color;
layout(location = 1) out vec2 f_uv;
layout(location = 2) flat out int f_sampler_index;

void main()
{
    gl_Position = vec4(v_position, 0.0, 1.0);
    f_color = v_color.zyxw;
    f_uv = v_uv;
    f_sampler_index = v_sampler_index;
}
