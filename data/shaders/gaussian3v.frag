uniform sampler2D tex;
uniform vec2 pixel;

// Gaussian separated blur with radius 3.

void main()
{
	vec4 sum = vec4(0.0);
	float X = gl_TexCoord[0].x;
	float Y = gl_TexCoord[0].y;

	sum += texture2D(tex, vec2(X, Y - 3.0 * pixel.y)) * 0.03125;
	sum += texture2D(tex, vec2(X, Y - 1.3333 * pixel.y)) * 0.328125;
	sum += texture2D(tex, vec2(X, Y)) * 0.273438;
	sum += texture2D(tex, vec2(X, Y + 1.3333 * pixel.y)) * 0.328125;
	sum += texture2D(tex, vec2(X, Y + 3.0 * pixel.y)) * 0.03125;

	gl_FragColor = sum;
}
