uniform sampler2D tex;
uniform vec3 ambient;
//uniform sampler2D spectex;

void main()
{
	vec2 texc = gl_TexCoord[0].xy;

	vec4 col = texture2D(tex, texc);
	//vec4 specular = texture2D(spectex, texc);

	col.xyz += ambient;
	float spec = col.a - 0.05;
	//spec *= specular.a;
	col.xyz += spec * col.xyz;
	col.a = 1.0;

	gl_FragColor = col;
}
