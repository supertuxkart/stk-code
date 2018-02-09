in vec3 normal;
in vec2 uv;
in vec2 uv_two;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec4 o_normal_color;

#stk_include "utils/encode_normal.frag"
#stk_include "utils/sp_texture_sampling.frag"

void main(void)
{
    vec4 color = sampleTextureLayer0(uv);
    vec4 layer_two_tex = sampleTextureLayer1(uv_two);
    layer_two_tex.rgb = layer_two_tex.a * layer_two_tex.rgb;

    vec3 final_color = layer_two_tex.rgb + color.rgb * (1.0 - layer_two_tex.a);

#if defined(Advanced_Lighting_Enabled)
    vec4 layer_2 = sampleTextureLayer2(uv);
    o_diffuse_color = vec4(final_color, layer_2.z);

    o_normal_color.xy = 0.5 * EncodeNormal(normalize(normal)) + 0.5;
    o_normal_color.zw = layer_2.xy;
#else
    o_diffuse_color = vec4(final_color, 1.0);
#endif

}
