#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D Albedo;
layout(bindless_sampler) uniform sampler2D SpecMap;
layout(bindless_sampler) uniform sampler2D colorization_mask;
#else
uniform sampler2D Albedo;
uniform sampler2D SpecMap;
uniform sampler2D colorization_mask;
#endif

uniform vec2 color_change;

in vec2 uv;
in vec4 color;
out vec4 FragColor;

#stk_include "utils/getLightFactor.frag"
#stk_include "utils/rgb_conversion.frag"

void main(void)
{
    vec4 col = texture(Albedo, uv);
#ifdef Use_Bindless_Texture
#ifdef SRGBBindlessFix
    col.xyz = pow(col.xyz, vec3(2.2));
#endif
#endif
    if (col.a * color.a < 0.5) discard;
    
#if defined(sRGB_Framebuffer_Usable) || defined(Advanced_Lighting_Enabled)
    col.xyz *= pow(color.xyz, vec3(2.2));
#else
    col.xyz *= color.xyz;
#endif

    float mask = texture(colorization_mask, uv).a;
    if (color_change.x > 0.0)
    {
        vec3 old_hsv = rgbToHsv(col.rgb);
        float mask_step = step(mask, 0.5);
        vec2 new_xy = mix(vec2(old_hsv.x, old_hsv.y), vec2(color_change.x, max(old_hsv.y, color_change.y)), vec2(mask_step, mask_step));
        col.xyz = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
    }

    float specmap = texture(SpecMap, uv).g;
    float emitmap = texture(SpecMap, uv).b;
    
    FragColor = vec4(getLightFactor(col.xyz, vec3(1.), specmap, emitmap), 1.);
}
