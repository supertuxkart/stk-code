#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D Albedo;
layout(bindless_sampler) uniform sampler2D dtex;
layout(bindless_sampler) uniform sampler2D SpecMap;
layout(bindless_sampler) uniform sampler2D colorization_mask;
#else
uniform sampler2D Albedo;
uniform sampler2D dtex;
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

    vec2 texc = gl_FragCoord.xy / screen;
    float z = texture(dtex, texc).x;

    vec4 xpos = 2.0 * vec4(texc, z, 1.0) - 1.0f;
    xpos = InverseProjectionMatrix * xpos;
    xpos /= xpos.w;
    vec3 eyedir = normalize(xpos.xyz);

    // Inspired from http://http.developer.nvidia.com/GPUGems3/gpugems3_ch16.html
    vec3 L = normalize((transpose(InverseViewMatrix) * vec4(sun_direction, 0.)).xyz);
    float fEdotL = clamp(dot(L, eyedir), 0., 1.);
    float fPowEdotL = pow(fEdotL, 4.);

    float fLdotNBack  = max(0., - dot(nor, L) * 0.6 + 0.4);
    float scattering = mix(fPowEdotL, fLdotNBack, .5);

    float specmap = texture(SpecMap, uv).g;
    vec3 LightFactor = color.xyz * (scattering * 0.1) + getLightFactor(color.xyz, vec3(1.), specmap, 0.);
    FragColor = vec4(LightFactor, 1.);
}
