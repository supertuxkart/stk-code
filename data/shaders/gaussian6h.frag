#version 330
uniform sampler2D tex;
uniform vec2 pixel;

// Gaussian separated blur with radius 6.

in vec2 uv;
out vec4 FragColor;

void main()
{
	vec4 sum = vec4(0.0);
	float X = uv.x;
	float Y = uv.y;

	sum += texture(tex, vec2(X - 5.13333 * pixel.x, Y)) * 0.00640869;
	sum += texture(tex, vec2(X - 3.26667 * pixel.x, Y)) * 0.083313;
	sum += texture(tex, vec2(X - 1.4 * pixel.x, Y)) * 0.305481;
	sum += texture(tex, vec2(X, Y)) * 0.209473;
	sum += texture(tex, vec2(X + 1.4 * pixel.x, Y)) * 0.305481;
	sum += texture(tex, vec2(X + 3.26667 * pixel.x, Y)) * 0.083313;
	sum += texture(tex, vec2(X + 5.13333 * pixel.x, Y)) * 0.00640869;

	FragColor = sum;
}
