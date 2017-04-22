uniform sampler2D tex;
uniform vec2 pixel;
uniform float sigma;

// Gaussian separated blur with radius 6.

out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy * pixel;
    float X = uv.x;
    float Y = uv.y;

    float g0, g1, g2;
    g0 = 1.0 / (sqrt(2.0 * 3.14) * sigma);
    g1 = exp(-0.5 / (sigma * sigma));
    g2 = g1 * g1;
    vec4 sum = texture(tex, vec2(X, Y)) * g0;
    g0 *= g1;
    g1 *= g2;
    for (int i = 1; i < 6; i++) {
        sum += texture(tex, vec2(X, Y - float(i) * pixel.y)) * g0;
        sum += texture(tex, vec2(X, Y + float(i) * pixel.y)) * g0;
        g0 *= g1;
        g1 *= g2;
    }

    FragColor = sum;
}
