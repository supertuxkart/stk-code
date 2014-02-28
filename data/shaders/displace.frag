uniform sampler2D tex;
uniform vec2 dir;
uniform vec2 dir2;

#if __VERSION__ >= 130
in vec2 uv;
in vec2 uv_bis;
in float camdist;

out vec4 FragColor;
#else
varying vec2 uv;
varying vec2 uv_bis;
varying float camdist;
#define FragColor gl_FragColor
#endif

const float maxlen = 0.02;

void main()
{
	float horiz = texture(tex, uv + dir).x;
	float vert = texture(tex, (uv.yx + dir2) * vec2(0.9)).x;

	vec2 offset = vec2(horiz, vert);
	offset *= 2.0;
	offset -= 1.0;

	// Fade according to distance to cam
	float fade = 1.0 - smoothstep(1.0, 100.0, camdist);

	// Fade according to distance from the edges
	const float mindist = 0.1;
	fade *= smoothstep(0.0, mindist, uv_bis.x) * smoothstep(0.0, mindist, uv_bis.y) *
		(1.0 - smoothstep(1.0 - mindist, 1.0, uv_bis.x)) *
		(1.0 - smoothstep(1.0 - mindist, 1.0, uv_bis.y));

	offset *= 50.0 * fade * maxlen;

	vec4 col;
	col.r = step(offset.x, 0.0) * -offset.x;
	col.g = step(0.0, offset.x) * offset.x;
	col.b = step(offset.y, 0.0) * -offset.y;
	col.a = step(0.0, offset.y) * offset.y;

	FragColor = col;
}
