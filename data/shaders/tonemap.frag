// From http://www.ceng.metu.edu.tr/~akyuz/files/hdrgpu.pdf

uniform sampler2D tex;
uniform sampler2D logluminancetex;
uniform float exposure = .09;
uniform float Lwhite = 1.;
uniform float vignette_weight = 0.;

out vec4 FragColor;

vec3 getCIEYxy(vec3 rgbColor);
vec3 getRGBFromCIEXxy(vec3 YxyColor);


float delta = .0001;
float saturation = 1.;

void main()
{
    vec2 uv = gl_FragCoord.xy / screen;
    vec4 col = texture(tex, uv);
    float avgLw = textureLod(logluminancetex, uv, 10.).x;
    avgLw = max(exp(avgLw) - delta, delta);

    vec3 Cw = getCIEYxy(col.xyz);
    float Lw = Cw.y;

    /* Reinhard, for reference */
//    float L = Lw * exposure / avgLw;
//    float Ld = L * (1. + L / (Lwhite * Lwhite));
//    Ld /= (1. + L);
//    FragColor = vec4(Ld * pow(col.xyz / Lw, vec3(saturation)), 1.);

    // Uncharted2 tonemap with Auria's custom coefficients
    vec4 perChannel = (col * (6.9 * col + .5)) / (col * (5.2 * col + 1.7) + 0.06);
    perChannel = pow(perChannel, vec4(2.2));

    vec2 inside = uv - 0.5;
    float vignette = 1. - dot(inside, inside) * vignette_weight;
    vignette = clamp(pow(vignette, 0.8), 0., 1.);
    //vignette = clamp(vignette + vignette - 0.5, 0., 1.15);

    FragColor = vec4(perChannel.xyz * vignette, col.a);
}
