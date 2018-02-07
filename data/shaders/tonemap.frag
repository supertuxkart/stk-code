uniform sampler2D tex;
uniform float vignette_weight;

out vec4 FragColor;

#stk_include "utils/getCIEXYZ.frag"
#stk_include "utils/getRGBfromCIEXxy.frag"

void main()
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    vec4 col = texture(tex, uv);

    // Uncharted2 tonemap with Auria's custom coefficients
    vec4 perChannel = (col * (6.9 * col + .5)) / (col * (5.2 * col + 1.7) + 0.06);
    vec2 inside = uv - 0.5;
    float vignette = 1. - dot(inside, inside) * vignette_weight;
    vignette = clamp(pow(vignette, 0.8), 0., 1.);

    FragColor = vec4(perChannel.xyz * vignette, col.a);
}
