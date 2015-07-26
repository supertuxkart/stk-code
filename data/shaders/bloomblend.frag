uniform sampler2D tex_128;
uniform sampler2D tex_256;
uniform sampler2D tex_512;

uniform sampler2D tex_dust;

out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / screen;
    vec4 col = .125 * texture(tex_128, uv);
    col += .25 * texture(tex_256, uv);
    col += .5 * texture(tex_512, uv);

    col = texture(tex_dust, uv);

    FragColor = vec4(col.xyz, 1.);
}
