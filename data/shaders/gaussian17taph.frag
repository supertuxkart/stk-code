// From http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

uniform sampler2D tex;
uniform vec2 pixel;


in vec2 uv;
out vec4 FragColor;

void main()
{

    float X = uv.x;
    float Y = uv.y;

    float offset[5] = {
        0.,
        1.41176470588235,
        3.29411764705882,
        5.17647058823529,
        7.05882352941176
    };
    float weight[5] = {
        0.196380615234375,
        0.1888427734375,
        0.03631591796875,
        0.0020751953125,
        0.000015258789062
    };

    vec4 sum = texture(tex, vec2(X, Y)) * weight[0];
    for (int i = 1; i < 5; i++) {
        sum += texture(tex, vec2(X - offset[i] * pixel.x, Y)) * weight[i];
        sum += texture(tex, vec2(X + offset[i] * pixel.x, Y)) * weight[i];
    }

    FragColor = sum;
}
