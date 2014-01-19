#version 130
uniform sampler2D tex;
uniform vec2 screen;
uniform vec2 dir;
uniform vec2 dir2;

in vec2 uv;
in vec2 edger_uv;
in float camdist;

out vec4 FragColor;

void main()
{
	vec2 tc = uv;

	vec4 col = vec4(0.0);
	const float maxlen = 0.02;

	float horiz = texture(tex, tc + dir).x;
	float vert = texture(tex, (tc.yx + dir2) * vec2(0.9)).x;

	vec2 offset = vec2(horiz, vert);
	offset *= 2.0;
	offset -= 1.0;

	// Fade according to distance to cam
	float fade = 1.0 - smoothstep(1.0, 100.0, camdist);

	// Fade according to distance from the edges
	vec2 edger = edger_uv;
	const float mindist = 0.1;
	fade *= smoothstep(0.0, mindist, edger.x) * smoothstep(0.0, mindist, edger.y) *
		(1.0 - smoothstep(1.0 - mindist, 1.0, edger.x)) *
		(1.0 - smoothstep(1.0 - mindist, 1.0, edger.y));

	offset *= 50.0 * fade * maxlen;

	col.r = step(offset.x, 0.0) * -offset.x;
	col.g = step(0.0, offset.x) * offset.x;
	col.b = step(offset.y, 0.0) * -offset.y;
	col.a = step(0.0, offset.y) * offset.y;

	FragColor = col;
}
