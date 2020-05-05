uniform sampler2D tex;

in vec2 uv;
in vec4 color;
out vec4 FragColor;

void main()
{
    vec4 res = texture(tex, uv);
    FragColor = res * color;
}
