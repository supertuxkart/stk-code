in vec3 bitangent;
in vec4 color;
in float hue_change;
in vec3 normal;
in vec3 tangent;
in vec2 uv;
in float camdist;
in vec4 world_position;
in vec3 world_normal;

layout(location = 0) out vec4 o_diffuse_color;
layout(location = 1) out vec4 o_normal_color;

#stk_include "utils/encode_normal.frag"
#stk_include "utils/rgb_conversion.frag"
#stk_include "utils/sp_texture_sampling.frag"

float overlay(float bg, float fg)
{
    return bg < 0.5 ? (2.0 * bg * fg) : (1.0 - 2.0 * (1.0 - bg) * (1.0 - fg));
}

void main()
{
    vec4 col = multi_sampleTextureLayer0(uv, camdist);
    vec4 rock = multi_sampleTextureLayer4(uv, camdist);
    
    vec2 uuv = vec2(world_position.x, world_position.z);


    float mask_2 = multi_sampleTextureLayer5(uv, camdist).r;
    mask_2 = pow(mask_2, 1.5);

    float up_mask = dot(vec3(0., 1., 0.), world_normal);
    up_mask = pow(up_mask + (up_mask * 0.1), 2);
    up_mask = clamp(up_mask, 0.0, 1.0);
    //vec3 final_color = mix(col.rgb, rock.rgb, overlay(color.r,  mask_2));
    vec3 final_color = mix(rock.rgb, col.rgb, overlay(up_mask,  mask_2));
    //vec3 final_color = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), overlay(color.r,  mask_2));

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
