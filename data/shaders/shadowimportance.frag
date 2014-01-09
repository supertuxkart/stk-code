#version 130
uniform sampler2D ntex;
uniform sampler2D ctex;
uniform vec3 campos;
uniform int low;

in vec3 wpos;
in vec2 texc;

float luminanceImp()
{
	// A full-res fetch kills on low-end
	if (low > 0) return 1.0;

	const vec3 weights = vec3(0.2126, 0.7152, 0.0722); // ITU-R BT. 709
	vec3 col = texture2D(ctex, texc).xyz;

	float luma = dot(weights, col);

	// Dark surfaces need less resolution
	float f = smoothstep(0.1, 0.4, luma);
	f = max(0.05, f);

	return f;
}

float normalImp(vec3 normal)
{
	vec3 camdir = normalize(campos - wpos);
	vec3 N = normalize(normal);

	// Boost surfaces facing the viewer directly
	float f = 2.0 * max(0.0, dot(N, camdir));

	return f;
}

float depthImp(float linearz)
{
/*	const float skip = 0.7;

	float f = min(linearz, skip);
	f *= 1.0/skip;*/

	float z = log(1.0 + linearz * 9.0) / log(10.0);

	float f = 1.0 - (z * 0.9);

	return f;
}

void main()
{
	vec4 ntmp = texture2D(ntex, texc);
	vec3 normal = ntmp.xyz * 2.0 - 1.0;
	float linearz = ntmp.a;

	float importance = normalImp(normal) * depthImp(linearz) * luminanceImp();
	importance = clamp(importance, 0.0, 1.0);

	float low = step(0.001, importance);

	// Quantize it
	const float steps = 16.0;
	importance *= steps;
	importance = ceil(importance) * low;
	importance /= steps;

	gl_FragColor = vec4(importance);
	gl_FragDepth = 1.0 - importance;
}
