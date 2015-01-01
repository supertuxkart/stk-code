uniform sampler2D tex;
uniform vec3 col;

in vec2 uv;
out vec4 FragColor;

void main()
{
	vec4 res = texture(tex, uv);

	// Keep the sun fully bright, but fade the sky
	float mul = distance(res.xyz, col);
	mul = step(mul, 0.02);
	mul *= 0.97;

	res = res * vec4(mul);

	FragColor = res;
}
