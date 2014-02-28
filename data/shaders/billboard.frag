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
    FragColor = texture(tex, uv);
}
