uniform sampler2D tex;

void main()
{
	vec4 col = texture2D(tex, gl_TexCoord[0].xy);

	col.xyz *= 10.0 * col.a;

	gl_FragColor = vec4(col.xyz, 1.0);
}
