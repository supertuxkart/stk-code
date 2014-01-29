#version 130
uniform sampler2D tex;
in vec2 uv;
out vec4 FragColor;

void main(void)
{
    vec4 color = texture(tex, uv);
    if (color.a < 0.5) discard;
    FragColor = vec4(color.xyz, 1.);
}
