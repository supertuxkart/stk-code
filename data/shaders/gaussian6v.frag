uniform sampler2D tex;
uniform vec2 pixel;

// Gaussian separated blur with radius 6.

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif

void main()
{
	vec4 sum = vec4(0.0);
	float X = uv.x;
	float Y = uv.y;

	sum += texture(tex, vec2(X, Y - 5.13333 * pixel.y)) * 0.00640869;
	sum += texture(tex, vec2(X, Y - 3.26667 * pixel.y)) * 0.083313;
	sum += texture(tex, vec2(X, Y - 1.4 * pixel.y)) * 0.305481;
	sum += texture(tex, vec2(X, Y)) * 0.209473;
	sum += texture(tex, vec2(X, Y + 1.4 * pixel.y)) * 0.305481;
	sum += texture(tex, vec2(X, Y + 3.26667 * pixel.y)) * 0.083313;
	sum += texture(tex, vec2(X, Y + 5.13333 * pixel.y)) * 0.00640869;

	FragColor = sum;
}
