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
	FragColor = texture(tex, uv) * color;
}
