uniform sampler2D road_masking;

in vec3 bitangent;
in vec4 color;
in float hue_change;
in vec3 normal;
in vec3 tangent;
in vec2 uv;
in vec4 world_position;
in float camdist;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec4 o_normal_color;

#stk_include "utils/encode_normal.frag"
#stk_include "utils/rgb_conversion.frag"
#stk_include "utils/sp_texture_sampling.frag"

void main()
{
    vec2 uuv = vec2(world_position.x, world_position.z);
    uuv *= 0.2;
    vec4 col = multi_sampleTextureLayer0(uv, camdist);

    float mask = sampleTextureLayer4(uuv * 2.0).r;
    mask += sampleTextureLayer4(uuv * 0.5).r;

    //* (1.0 - color.g)
    mask = mix(1.0, mask, color.r);
    mask = mix(0.0, mask, 1.0 - color.g);
    if(mask < 0.5)
    {
        discard;
    }

    // Adding some skidding marks to the road
    float mask_2 = sampleTextureLayer4(uuv * 0.1).r;
    float mask_3 = sampleTextureLayer4(uuv * 3.5).r;
    mask_2 = pow(mask_2, 1.5);
    mask_2 *= pow(mask_3, 0.5);

    float skidding_marks = texture(road_masking, uv * 10.0).g;
    skidding_marks *= mask_2;
    col = mix(col, vec4(0.0, 0.0, 0.0, 1.0), skidding_marks);

    float skidding_marks_2 = texture(road_masking, uv * 15.0).g;
    skidding_marks_2 *= mask_2;
    col = mix(col, vec4(0.0, 0.0, 0.0, 1.0), skidding_marks_2);

    // Add some cracks
    float cracks_marks = texture(road_masking, uv * 11.0).b;
    float crack_mask = sampleTextureLayer4(uuv * 0.5).r;
    cracks_marks *= crack_mask;
    col = mix(col, vec4(0.0, 0.0, 0.0, 1.0), cracks_marks);

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

    vec3 final_color = col.xyz; // * color.xyz;

#if defined(Advanced_Lighting_Enabled)
    vec4 layer_2 = multi_sampleTextureLayer2(uv, camdist);
    vec4 layer_3 = multi_sampleTextureLayer3(uv, camdist);
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
