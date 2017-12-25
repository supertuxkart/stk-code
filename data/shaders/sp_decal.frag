#ifdef Use_Bindless_Texture
flat in sampler2D tex_layer_0;
flat in sampler2D tex_layer_1;
flat in sampler2D tex_layer_2;
#else
// spm layer 1 texture
uniform sampler2D tex_layer_0;
// spm layer 1 texture
uniform sampler2D tex_layer_1;
// gloss map
uniform sampler2D tex_layer_2;
#endif

#ifdef Use_Array_Texture
uniform sampler2DArray tex_array;
flat in float array_0;
flat in float array_1;
flat in float array_2;
#endif

in vec3 normal;
in vec2 uv;
in vec2 uv_two;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec3 o_normal_depth;
layout(location = 2) out vec2 o_gloss_map;

#stk_include "utils/encode_normal.frag"

void main(void)
{
#ifdef Use_Array_Texture
    vec4 color = texture(tex_array, vec3(uv, array_0));
    vec4 layer_two_tex = texture(tex_array, vec3(uv_two, array_1));
#else
    vec4 color = texture(tex_layer_0, uv);
    vec4 layer_two_tex = texture(tex_layer_1, uv_two);
#endif

    layer_two_tex.rgb = layer_two_tex.a * layer_two_tex.rgb;

    vec3 final_color = layer_two_tex.rgb + color.rgb * (1.0 - layer_two_tex.a);
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
    vec4 layer_2 = texture(tex_array, vec3(uv, array_2));
#else
    vec4 layer_2 = texture(tex_layer_2, uv);
#endif

    o_normal_depth.xy = 0.5 * EncodeNormal(normalize(normal)) + 0.5;
    o_normal_depth.z = layer_2.x;
    o_gloss_map = layer_2.yz;

#endif
}

