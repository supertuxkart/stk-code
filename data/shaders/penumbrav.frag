#version 330
uniform sampler2D tex;
uniform vec2 pixel;

in vec2 uv;
out vec4 FragColor;

// Separated penumbra, vertical

void main()
{
	float sum = 0.0;
	vec4 tmp;
	float X = uv.x;
	float Y = uv.y;
	float width = 0.0;
	float zsum = 0.00001;

	tmp = texture(tex, vec2(X, Y - 5.13333 * pixel.y));
	sum += tmp.x * 0.00640869;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture(tex, vec2(X, Y - 3.26667 * pixel.y));
	sum += tmp.x * 0.083313;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture(tex, vec2(X, Y - 1.4 * pixel.y));
	sum += tmp.x * 0.305481;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture(tex, vec2(X, Y));
	sum += tmp.x * 0.209473;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture(tex, vec2(X, Y + 1.4 * pixel.y));
	sum += tmp.x * 0.305481;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture(tex, vec2(X, Y + 3.26667 * pixel.y));
	sum += tmp.x * 0.083313;
	zsum += tmp.z;
	width += tmp.y;

	tmp = texture(tex, vec2(X, Y + 5.13333 * pixel.y));
	sum += tmp.x * 0.00640869;
	zsum += tmp.z;
	width += tmp.y;

	float hasz = step(0.7, zsum);
	FragColor = vec4(sum, (width / zsum) * hasz, hasz, 1.0);
}
