uniform sampler2D tex;

in vec2 uv;
in vec4 color;
out vec4 FragColor;


void main()
{
    vec4 Color = texture(tex, uv);
    Color.xyz *= pow(color.xyz, vec3(2.2));
    Color.a *= color.a;
    // Premultiply alpha
    FragColor = vec4(Color.rgb * Color.a, Color.a);
}
