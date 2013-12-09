uniform sampler2D tex;
uniform float time;

void main()
{
	vec2 change;
	change.x = abs(sin(time * gl_Color.r));
	change.y = abs(cos(time * gl_Color.g));

	change = smoothstep(0.0, 1.0, change) * 0.5;

	vec2 tc = gl_TexCoord[0].xy;
	tc = smoothstep(0.5 - change, 0.5 + change, tc);

	vec4 tcol = texture2D(tex, tc);

	gl_FragColor = tcol;
}
