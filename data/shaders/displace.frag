uniform sampler2D displacement_tex;
uniform sampler2D mask_tex;
uniform sampler2D color_tex;
uniform sampler2D tex;
uniform vec2 dir;
uniform vec2 dir2;

in vec2 uv;
in float camdist;

out vec4 FragColor;

const float maxlen = 0.02;

void main()
{
	float horiz = texture(displacement_tex, uv + dir).x;
	float vert = texture(displacement_tex, (uv.yx + dir2) * vec2(0.9)).x;

	vec2 offset = vec2(horiz, vert);
	offset *= 2.0;
	offset -= 1.0;

	// Fade according to distance to cam
	float fade = 1.0 - smoothstep(1.0, 100.0, camdist);

	vec4 shiftval;
	shiftval.r = step(offset.x, 0.0) * -offset.x;
	shiftval.g = step(0.0, offset.x) * offset.x;
	shiftval.b = step(offset.y, 0.0) * -offset.y;
	shiftval.a = step(0.0, offset.y) * offset.y;

	vec2 shift;
	shift.x = -shiftval.x + shiftval.y;
	shift.y = -shiftval.z + shiftval.w;
	shift /= 50.;

	vec2 tc = gl_FragCoord.xy / screen;
	float mask = texture(mask_tex, tc + shift).x;
	tc += (mask < 1.) ? vec2(0.) : shift;

    vec4 col = texture(color_tex, tc);
    vec4 blend_tex = texture(tex, uv);
    col.rgb = blend_tex.rgb * blend_tex.a + (1. - blend_tex.a) * col.rgb;
    FragColor = vec4(col.rgb, 1.);
}
