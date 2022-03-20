layout(location = 0) in vec4 f_color;
layout(location = 1) in vec2 f_uv;
layout(location = 2) flat in int f_sampler_index;

#ifdef BIND_TEXTURES_AT_ONCE
layout(binding = 0) uniform sampler2D f_tex[SAMPLER_SIZE];
#else
layout(binding = 0) uniform sampler2D f_tex;
#endif

layout(location = 0) out vec4 o_color;

void main()
{
#ifdef BIND_TEXTURES_AT_ONCE
    vec4 tex_color = texture(f_tex[GE_SAMPLE_TEX_INDEX(f_sampler_index)], f_uv);
#else
    vec4 tex_color = texture(f_tex, f_uv);
#endif
    o_color = tex_color * f_color;
}
