uniform float power;
uniform sampler2D tex;

void main()
{
	vec4 col = texture2D(tex, gl_TexCoord[0].xy);
	if (col.a < 0.5)
		discard;

	gl_FragColor = vec4(col.xyz, power);
}
