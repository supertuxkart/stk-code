uniform sampler2D tex;
uniform int size;
uniform vec2 pixel;

vec4 encdepth(float v) {
	vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
	enc = fract(enc);
	enc -= enc.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
	return enc;
}

void main()
{
	vec2 origtc = gl_TexCoord[0].xy;

	// Get total sum
	float first = 1.0, last = 0.0;
	float lower = 0.0;
	float total = 0.0;
	vec2 tc = 0.5 * pixel;

	for (int i = 0; i < size; i++)
	{
		float col = texture2D(tex, tc).x;

		lower += col * step(tc.x, origtc.x);
		total += col;

		if (col > 0.0001)
		{
			first = min(first, tc.x);
			last = max(last, tc.x);
		}

		tc += pixel;
	}

	float res = (lower / total) - origtc.x;

	// Outside the edges?
	if (origtc.x <= first)
	{
		res = origtc.x * -2.1;
	}
	else if (origtc.x >= last)
	{
		res = (1.0 - origtc.x) * 2.1;
	}

	res = res * 0.5 + 0.5;
	res = clamp(res, 0.01, 0.99);

	gl_FragColor = encdepth(res);
}
