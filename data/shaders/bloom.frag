uniform sampler2D tex;
uniform float low;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif

vec3 getCIEYxy(vec3 rgbColor);

void main()
{
	vec3 col = texture(tex, uv).xyz;
	float luma = getCIEYxy(col).x;

	col *= smoothstep(1., 10., luma);

	FragColor = vec4(col, 1.0);
}
