uniform sampler2D tex;

in vec2 uv;
in vec4 vertex_color;
out vec4 FragColor;

void main(void)
{
    vec4 color = texture(tex, uv);
    color *= vertex_color;
    FragColor = vec4(color.a * color.rgb, color.a);
}
