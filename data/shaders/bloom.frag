uniform sampler2D tex;
uniform float low;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif


void main()
{
	vec3 weights = vec3(0.2126, 0.7152, 0.0722); // ITU-R BT. 709
	vec3 col = texture(tex, uv).xyz;
	float luma = dot(weights, col);

	col *= smoothstep(1., 2., luma);

	FragColor = vec4(col, 1.0);
}
