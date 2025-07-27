uniform sampler2D displacement_tex;
uniform sampler2D mask_tex;
uniform sampler2D color_tex;
uniform sampler2D ssr_tex;

#stk_include "utils/displace_utils.frag"
#stk_include "utils/sp_texture_sampling.frag"

uniform vec4 direction;
uniform int u_ssr;

in vec2 uv;

out vec4 o_diffuse_color;

void main()
{
    float horiz = texture(displacement_tex, uv + direction.xy * 150.).x;
    float vert = texture(displacement_tex, (uv.yx + direction.zw * 150.) * vec2(0.9)).x;
    vec2 shift = getDisplaceShift(horiz, vert);
    shift *= 0.02;

    vec2 tc = gl_FragCoord.xy / u_screen;
    float mask = texture(mask_tex, tc + shift).x;
    tc += (mask < 1.) ? vec2(0.) : shift;

    vec4 blend_tex = sampleTextureLayer0(uv);
    vec3 col = blend_tex.rgb * blend_tex.a;
    if (u_ssr == 1)
    {
        col.rgb *= 0.7;
        col.rgb += texture(ssr_tex, tc).rgb * blend_tex.a * 0.3;
    }
    col.rgb += (1. - blend_tex.a) * texture(color_tex, tc).rgb;
    o_diffuse_color = vec4(col.rgb, 1.);
}
