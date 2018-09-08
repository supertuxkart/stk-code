in vec3 bitangent;
in vec4 color;
in float hue_change;
in vec3 normal;
in vec3 tangent;
in vec2 uv;
in vec4 world_position;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec4 o_normal_color;

#stk_include "utils/encode_normal.frag"
#stk_include "utils/rgb_conversion.frag"
#stk_include "utils/sp_texture_sampling.frag"

uniform sampler2D g_detail_map;

void main()
{
    // Triplanar detail map
    vec2 xyuv = vec2(world_position.x, world_position.y);
    float detail = texture(g_detail_map, uv * 32.0).r;
    detail *= 2.5;

    vec4 col = sampleTextureLayer0(uv);
    col = (col - detail) + 0.5;
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

#if defined(Advanced_Lighting_Enabled)
    vec4 layer_2 = sampleTextureLayer2(uv);
    vec4 layer_3 = sampleTextureLayer3(uv);
    o_diffuse_color = vec4(final_color, layer_2.z);

    vec3 tangent_space_normal = 2.0 * layer_3.xyz - 1.0;
    vec3 frag_tangent = normalize(tangent);
    vec3 frag_bitangent = normalize(bitangent);
    vec3 frag_normal = normalize(normal);
    mat3 t_b_n = mat3(frag_tangent, frag_bitangent, frag_normal);

    vec3 world_normal = t_b_n * tangent_space_normal;

    o_normal_color.xy = 0.5 * EncodeNormal(normalize(world_normal)) + 0.5;
    o_normal_color.zw = layer_2.xy;
#else
    o_diffuse_color = vec4(final_color, 1.0);
#endif
}
