#version 130
uniform sampler2D tex;
uniform vec3 col;

void main()
{
	vec4 res = texture(tex, gl_TexCoord[0].xy);

	// Keep the sun fully bright, but fade the sky
	float mul = distance(res.xyz, col);
	mul = step(mul, 0.02);
	mul *= 0.97;

	res = res * vec4(mul);

	gl_FragColor = res;
}
