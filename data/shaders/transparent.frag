uniform sampler2D tex;

#if __VERSION__ >= 130
in vec2 uv;
in vec4 color;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif


void main()
{
    vec4 Color = texture(tex, uv) * pow(color, vec4(2.2));
    // Premultiply alpha
    FragColor = vec4(Color.rgb * Color.a, Color.a);
}
