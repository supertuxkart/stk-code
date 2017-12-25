#ifdef Use_Bindless_Texture
flat in sampler2D tex_layer_0;
flat in sampler2D tex_layer_2;
flat in sampler2D tex_layer_3;
#else
// spm layer 1 texture
uniform sampler2D tex_layer_0;
// gloss map
uniform sampler2D tex_layer_2;
// normal map
uniform sampler2D tex_layer_3;
#endif


#ifdef Use_Array_Texture
uniform sampler2DArray tex_array;
flat in float array_0;
flat in float array_2;
flat in float array_3;
#endif

flat in float hue_change;

in vec4 color;
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;
in vec2 uv;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec3 o_normal_depth;
layout(location = 2) out vec2 o_gloss_map;

#stk_include "utils/encode_normal.frag"
#stk_include "utils/rgb_conversion.frag"

void main()
{
#ifdef Use_Array_Texture
    vec4 col = texture(tex_array, vec3(uv, array_0));
#else
    vec4 col = texture(tex_layer_0, uv);
#endif

    if (hue_change > 0.0)
    {
        float mask = col.a;
        vec3 old_hsv = rgbToHsv(col.rgb);
        float mask_step = step(mask, 0.5);
#if !defined(Advanced_Lighting_Enabled)
        float saturation = mask * 2.1;
#else
        float saturation = mask * 2.5;
#endif
        vec2 new_xy = mix(vec2(old_hsv.x, old_hsv.y), vec2(hue_change,
            max(old_hsv.y, saturation)), vec2(mask_step, mask_step));
        vec3 new_color = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
        col = vec4(new_color.r, new_color.g, new_color.b, 1.0);
    }

    vec3 final_color = col.xyz * color.xyz;
#if !defined(Advanced_Lighting_Enabled)
#if !defined(sRGB_Framebuffer_Usable)
    final_color = final_color * 0.73; // 0.5 ^ (1. / 2.2)
#else
    final_color = final_color * 0.5;
#endif
#endif
    o_diffuse_color = vec4(final_color, 1.0);

#if defined(Advanced_Lighting_Enabled)
#ifdef Use_Array_Texture
    vec4 layer_3 = texture(tex_array, vec3(uv, array_3));
#else
    vec4 layer_3 = texture(tex_layer_3, uv);
#endif

    vec3 tangent_space_normal = 2.0 * layer_3.xyz - 1.0;
    vec3 frag_tangent = normalize(tangent);
    vec3 frag_bitangent = normalize(bitangent);
    vec3 frag_normal = normalize(normal);
    mat3 t_b_n = mat3(frag_tangent, frag_bitangent, frag_normal);
    vec3 world_normal = t_b_n * tangent_space_normal;

#ifdef Use_Array_Texture
    vec4 layer_2 = texture(tex_array, vec3(uv, array_2));
#else
    vec4 layer_2 = texture(tex_layer_2, uv);
#endif

    o_normal_depth.xy = 0.5 * EncodeNormal(normalize(world_normal)) + 0.5;
    o_normal_depth.z = layer_2.x;
    o_gloss_map = layer_2.yz;

#endif
}
