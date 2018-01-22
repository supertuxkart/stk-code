in vec3 normal;
in vec2 uv;
in vec2 uv_two;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec3 o_normal_depth;
layout(location = 2) out vec2 o_gloss_map;

#stk_include "utils/encode_normal.frag"
#stk_include "utils/sp_texture_sampling.frag"

void main(void)
{
    vec4 color = sampleTextureLayer0(uv);
    vec4 layer_two_tex = sampleTextureLayer1(uv_two);
    layer_two_tex.rgb = layer_two_tex.a * layer_two_tex.rgb;

    vec3 final_color = layer_two_tex.rgb + color.rgb * (1.0 - layer_two_tex.a);
#if !defined(Advanced_Lighting_Enabled)
    final_color = final_color * 0.73; // 0.5 ^ (1. / 2.2)
#endif
    o_diffuse_color = vec4(final_color, 1.0);

#if defined(Advanced_Lighting_Enabled)
    vec4 layer_2 = sampleTextureLayer2(uv);
    o_normal_depth.xy = 0.5 * EncodeNormal(normalize(normal)) + 0.5;
    o_normal_depth.z = layer_2.x;
    o_gloss_map = layer_2.yz;
#endif
}
