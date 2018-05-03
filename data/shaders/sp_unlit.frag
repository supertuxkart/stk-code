in vec4 color;
in vec3 normal;
in vec2 uv;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec4 o_normal_color;

#stk_include "utils/encode_normal.frag"
#stk_include "utils/sp_texture_sampling.frag"

void main(void)
{
    vec4 col = sampleTextureLayer0(uv);
    if (col.a < 0.5)
    {
        discard;
    }

    vec3 final_color = col.xyz * color.xyz;
    o_diffuse_color = vec4(final_color, 1.0);

#if defined(Advanced_Lighting_Enabled)
    o_diffuse_color = vec4(final_color, 0.4);

    o_normal_color.xy = 0.5 * EncodeNormal(normalize(normal)) + 0.5;
    o_normal_color.zw = vec2(0.0);
#else
    o_diffuse_color = vec4(final_color, 1.0);
#endif

}
