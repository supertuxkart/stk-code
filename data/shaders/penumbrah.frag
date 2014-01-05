#version 130
uniform sampler2D tex;
uniform vec2 pixel;

// Separated penumbra, horizontal

void main()
{
	float sum = 0.0;
	vec4 tmp;
	float X = gl_TexCoord[0].x;
	float Y = gl_TexCoord[0].y;
	float width = 0.0;
	float zsum = 0.00001;

	tmp = texture2D(tex, vec2(X - 5.13333 * pixel.x, Y));
	sum += tmp.x * 0.00640869;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture2D(tex, vec2(X - 3.26667 * pixel.x, Y));
	sum += tmp.x * 0.083313;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture2D(tex, vec2(X - 1.4 * pixel.x, Y));
	sum += tmp.x * 0.305481;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture2D(tex, vec2(X, Y));
	sum += tmp.x * 0.209473;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture2D(tex, vec2(X + 1.4 * pixel.x, Y));
	sum += tmp.x * 0.305481;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture2D(tex, vec2(X + 3.26667 * pixel.x, Y));
	sum += tmp.x * 0.083313;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture2D(tex, vec2(X + 5.13333 * pixel.x, Y));
	sum += tmp.x * 0.00640869;
	zsum += tmp.z;
	width += tmp.y;

	float hasz = step(0.7, zsum);
	gl_FragColor = vec4(sum, (width / zsum) * hasz, hasz, 1.0);
}
