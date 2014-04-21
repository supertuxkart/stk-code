#define AUTO_EXPOSURE

uniform sampler2D tex;
uniform sampler2D logluminancetex;

in vec2 uv;
out vec4 FragColor;

vec3 getCIEYxy(vec3 rgbColor);
vec3 getRGBFromCIEXxy(vec3 YxyColor);

float exposure = .3;
float whitePoint = 1.;
float delta = .1;

void main()
{
    vec4 col = texture(tex, uv);
    float avgLuminance = textureLod(logluminancetex, uv, 10.).x;
    avgLuminance = exp(avgLuminance) - delta;

    vec3 Yxy = getCIEYxy(col.xyz);
    float Lp = Yxy.r * exposure / avgLuminance;
    Yxy.r = (Lp * (1. * Lp / (whitePoint * whitePoint))) / (1. + Lp);
    FragColor = vec4(getRGBFromCIEXxy(Yxy), 1.);

}
