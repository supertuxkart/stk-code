layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_uv;
layout(location = 2) flat in int f_sampler_index;

layout(binding = 0) uniform sampler2D f_tex[SAMPLER_SIZE];

layout(location = 0) out vec4 o_color;

void main()
{
    o_color = texture(f_tex[GE_SAMPLE_TEX_INDEX(f_sampler_index)], f_uv) * f_color;
}
