uniform sampler2DArray tex;
uniform int layer;

in vec2 uv;
out vec4 FragColor;

void main()
{
    FragColor = texture(tex, vec3(uv, float(layer)));
}