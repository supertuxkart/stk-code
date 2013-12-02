uniform sampler2D tex1;
uniform sampler2D tex2;

void main()
{
	vec4 col1 = texture2D(tex1, gl_TexCoord[0].xy);
	vec4 col2 = vec4(vec3(texture2D(tex2, gl_TexCoord[0].xy).x), 1.0);

	gl_FragColor = col1 * col2;
}
