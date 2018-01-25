in vec2 uv;
in vec4 color;
out vec4 o_diffuse_color;

uniform sampler2D font_texture;

void main()
{
    vec4 col = texture(font_texture, uv);
    if (col.a < 0.5)
    {
        discard;
    }
    o_diffuse_color = vec4(col.xyz * color.xyz, 1.0);
}
