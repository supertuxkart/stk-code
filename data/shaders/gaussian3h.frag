uniform sampler2D tex;
uniform vec2 pixel;

// Gaussian separated blur with radius 3.

out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy * pixel;
    vec4 sum = vec4(0.0);
    float X = uv.x;
    float Y = uv.y;

    sum += texture(tex, vec2(X - 3.0 * pixel.x, Y)) * 0.03125;
    sum += texture(tex, vec2(X - 1.3333 * pixel.x, Y)) * 0.328125;
    sum += texture(tex, vec2(X, Y)) * 0.273438;
    sum += texture(tex, vec2(X + 1.3333 * pixel.x, Y)) * 0.328125;
    sum += texture(tex, vec2(X + 3.0 * pixel.x, Y)) * 0.03125;

    FragColor = sum;
}
