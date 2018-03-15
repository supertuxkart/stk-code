uniform float custom_alpha;

in float hue_change;

in vec2 uv;
in vec4 color;
out vec4 o_diffuse_color;

#stk_include "utils/rgb_conversion.frag"
#stk_include "utils/sp_texture_sampling.frag"

void main()
{
    vec4 col = sampleTextureLayer0(uv);
    if (hue_change > 0.0)
    {
        float mask = col.a;
        vec3 old_hsv = rgbToHsv(col.rgb);
        float mask_step = step(mask, 0.5);
#if !defined(Advanced_Lighting_Enabled)
        // For similar color
        float saturation = mask * 1.825; // 2.5 * 0.5 ^ (1. / 2.2)
#else
        float saturation = mask * 2.5;
#endif
        vec2 new_xy = mix(vec2(old_hsv.x, old_hsv.y), vec2(hue_change,
            max(old_hsv.y, saturation)), vec2(mask_step, mask_step));
        vec3 new_color = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
        col = vec4(new_color.r, new_color.g, new_color.b, 1.0);
    }

    vec3 final_color = col.xyz * color.xyz;
    o_diffuse_color = vec4((final_color * custom_alpha), custom_alpha);
}
