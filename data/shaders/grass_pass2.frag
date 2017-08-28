#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D Albedo;
layout(bindless_sampler) uniform sampler2D SpecMap;
layout(bindless_sampler) uniform sampler2D colorization_mask;
#else
uniform sampler2D Albedo;
uniform sampler2D SpecMap;
uniform sampler2D colorization_mask;
#endif

in vec3 nor;
in vec2 uv;
uniform vec2 color_change;
out vec4 FragColor;

#stk_include "utils/getLightFactor.frag"
#stk_include "utils/rgb_conversion.frag"

void main(void)
{
    vec4 color = texture(Albedo, uv);
#ifdef Use_Bindless_Texture
#ifdef SRGBBindlessFix
    color.xyz = pow(color.xyz, vec3(2.2));
#endif
#endif
    if (color.a < 0.5) discard;

    float mask = texture(colorization_mask, uv).a;
    if (color_change.x > 0.0)
    {
        vec3 old_hsv = rgbToHsv(color.rgb);
        float mask_step = step(mask, 0.5);
        vec2 new_xy = mix(vec2(old_hsv.x, old_hsv.y), vec2(color_change.x, max(old_hsv.y, color_change.y)), vec2(mask_step, mask_step));
        color.xyz = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
    }
    float specmap = texture(SpecMap, uv).g;
    float emitmap = texture(SpecMap, uv).b;
    vec3 LightFactor = getLightFactor(color.xyz, vec3(1.), specmap, emitmap);
    FragColor = vec4(LightFactor, 1.);
}
