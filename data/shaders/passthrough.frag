uniform sampler2D tex;
uniform int width;
uniform int height;

out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(width, height);
    FragColor = texture(tex, uv);
}