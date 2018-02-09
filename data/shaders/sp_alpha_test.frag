in float hue_change;

in vec4 color;
in vec3 normal;
in vec2 uv;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec3 o_normal_depth;
layout(location = 2) out vec2 o_gloss_map;

#stk_include "utils/encode_normal.frag"
#stk_include "utils/rgb_conversion.frag"
#stk_include "utils/sp_texture_sampling.frag"

void main(void)
{
    vec4 col = sampleTextureLayer0(uv);
    if (col.a * color.a < 0.5)
    {
        discard;
    }
    col.xyz *= color.xyz;

    if (hue_change > 0.0)
    {
        vec3 old_hsv = rgbToHsv(col.rgb);
        vec2 new_xy = vec2(hue_change, old_hsv.y);
        vec3 new_color = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
        col = vec4(new_color.r, new_color.g, new_color.b, col.a);
    }
    o_diffuse_color = vec4(col.xyz, 1.0);

#if defined(Advanced_Lighting_Enabled)
    vec4 layer_2 = sampleTextureLayer2(uv);
    o_normal_depth.xy = 0.5 * EncodeNormal(normalize(normal)) + 0.5;
    o_normal_depth.z = layer_2.x;
    o_gloss_map = layer_2.yz;
#endif
}
