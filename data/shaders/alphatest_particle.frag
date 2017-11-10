uniform sampler2D tex;

in vec2 tc;
in vec4 pc;
out vec4 FragColor;

void main(void)
{
    vec4 color = texture(tex, tc);
    if (color.a < 0.5)
    {
        discard;
    }
    FragColor = color * pc;
}
