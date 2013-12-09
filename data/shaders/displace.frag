uniform sampler2D tex;
uniform vec2 screen;
uniform vec2 dir;
uniform vec2 dir2;

varying float camdist;

void main()
{
	vec2 tc = gl_TexCoord[0].xy;

	vec4 col = vec4(0.0);
	const float maxlen = 0.02;

	float horiz = texture2D(tex, tc + dir).x;
	float vert = texture2D(tex, (tc.yx + dir2) * vec2(0.9)).x;

	vec2 offset = vec2(horiz, vert);
	offset *= 2.0;
	offset -= 1.0;

	// Fade according to distance to cam
	float fade = 1.0 - smoothstep(1.0, 40.0, camdist);

	// Fade according to distance from the edges
	vec2 edger = gl_TexCoord[1].xy;
	const float mindist = 0.1;
	fade *= smoothstep(0.0, mindist, edger.x) * smoothstep(0.0, mindist, edger.y) *
		(1.0 - smoothstep(1.0 - mindist, 1.0, edger.x)) *
		(1.0 - smoothstep(1.0 - mindist, 1.0, edger.y));

	offset *= 50.0 * fade * maxlen;

	col.r = step(offset.x, 0.0) * -offset.x;
	col.g = step(0.0, offset.x) * offset.x;
	col.b = step(offset.y, 0.0) * -offset.y;
	col.a = step(0.0, offset.y) * offset.y;

	gl_FragColor = col;
}
