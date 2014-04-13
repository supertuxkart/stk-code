uniform sampler2D tex;
uniform ivec4 color;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif

void main()
{
	vec4 res = texture(tex, uv);
	FragColor = vec4(res.xyz * color.xyz / 255., res.a);
}
