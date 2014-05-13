uniform sampler2D tex;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif


void main()
{
	vec4 col = texture(tex, uv);

	col.xyz *= 10.0 * col.a;

	FragColor = vec4(col.xyz, 1.);
}
