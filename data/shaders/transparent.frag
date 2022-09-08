uniform sampler2D tex;

in vec2 uv;
in vec4 color;
out vec4 FragColor;

void main()
{
    vec4 Color = texture(tex, uv);
    Color *= color;
    // Premultiply alpha
    FragColor = vec4(Color.rgb * Color.a, Color.a);
}
