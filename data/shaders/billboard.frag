#version 330
uniform sampler2D tex;

in vec2 uv;
out vec4 FragColor;

void main(void)
{
    FragColor = texture(tex, uv);
}
