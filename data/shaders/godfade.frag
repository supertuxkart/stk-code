uniform sampler2D tex;
uniform vec3 col;

out vec4 FragColor;

void main()
{
	// Use quarter resolution
	vec2 uv = 4. * gl_FragCoord.xy / u_screen;
	vec4 res = texture(tex, uv);

	// Keep the sun fully bright, but fade the sky
	float mul = distance(res.xyz, col);
	mul = step(mul, 0.02);
	mul *= 0.97;

	res = res * vec4(mul);

	FragColor = res;
}
