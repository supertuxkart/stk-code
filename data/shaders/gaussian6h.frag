uniform sampler2D tex;
uniform vec2 pixel;

// Gaussian separated blur with radius 6.

void main()
{
	vec4 sum = vec4(0.0);
	float X = gl_TexCoord[0].x;
	float Y = gl_TexCoord[0].y;

	sum += texture2D(tex, vec2(X - 5.13333 * pixel.x, Y)) * 0.00640869;
	sum += texture2D(tex, vec2(X - 3.26667 * pixel.x, Y)) * 0.083313;
	sum += texture2D(tex, vec2(X - 1.4 * pixel.x, Y)) * 0.305481;
	sum += texture2D(tex, vec2(X, Y)) * 0.209473;
	sum += texture2D(tex, vec2(X + 1.4 * pixel.x, Y)) * 0.305481;
	sum += texture2D(tex, vec2(X + 3.26667 * pixel.x, Y)) * 0.083313;
	sum += texture2D(tex, vec2(X + 5.13333 * pixel.x, Y)) * 0.00640869;

	gl_FragColor = sum;
}
