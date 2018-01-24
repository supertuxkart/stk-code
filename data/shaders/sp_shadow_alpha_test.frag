#stk_include "utils/sp_texture_sampling.frag"

in vec2 uv;
out vec4 o_frag_color;

void main(void)
{
    vec4 col = sampleTextureLayer0(uv);
    if (col.a < 0.5)
    {
        discard;
    }
    o_frag_color = vec4(1.0);
}
