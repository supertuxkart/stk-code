uniform sampler2D tex;

#if __VERSION__ >= 130
in vec2 uv;
in vec4 col;
out vec4 FragColor;
#else
varying vec2 uv;
varying vec4 col;
#define FragColor gl_FragColor
#endif



void main()
{
	vec4 res = texture(tex, uv);
	FragColor = vec4(res.xyz * col.xyz, res.a);
}
