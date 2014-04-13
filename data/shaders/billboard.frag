uniform sampler2D tex;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif


void main(void)
{
    vec4 color = texture(tex, uv);
    FragColor = vec4(color.rgb * color.a, color.a);
}
