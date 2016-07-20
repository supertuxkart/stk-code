// From http://http.developer.nvidia.com/GPUGems3/gpugems3_ch40.html

uniform sampler2D tex;
uniform sampler2D depth;
uniform vec2 pixel;

out vec4 FragColor;

void main()
{
    float sigma = 5.;

    vec2 uv = gl_FragCoord.xy * pixel;
    float X = uv.x;
    float Y = uv.y;

    float g0, g1, g2;
    g0 = 1.0 / (sqrt(2.0 * 3.14) * sigma);
    g1 = exp(-0.5 / (sigma * sigma));
    g2 = g1 * g1;
    vec4 sum = texture(tex, vec2(X, Y)) * g0;
    float pixel_depth = texture(depth, vec2(X, Y)).x;
    g0 *= g1;
    g1 *= g2;
    float tmp_weight, total_weight = g0;
    for (int i = 1; i < 9; i++) {
        tmp_weight = max(0.0, 1.0 - .001 * abs(texture(depth, vec2(X, Y - float(i) * pixel.y)).x - pixel_depth));
        sum += texture(tex, vec2(X, Y - float(i) * pixel.y)) * g0 * tmp_weight;
        total_weight += g0 * tmp_weight;
        tmp_weight = max(0.0, 1.0 - .001 * abs(texture(depth, vec2(X, Y + float(i) * pixel.y)).x - pixel_depth));
        sum += texture(tex, vec2(X, Y + float(i) * pixel.y)) * g0 * tmp_weight;
        total_weight += g0 * tmp_weight;
        g0 *= g1;
        g1 *= g2;
    }

    FragColor = sum / total_weight;
}

