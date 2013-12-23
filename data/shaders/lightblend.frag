uniform sampler2D diffuse_and_spec;
uniform sampler2D ambient_occlusion;
uniform vec3 ambient;
//uniform sampler2D spectex;

void main()
{
	vec2 texc = gl_TexCoord[0].xy;

	vec4 col = texture2D(diffuse_and_spec, texc);
	float ao = texture2D(ambient_occlusion, texc).x;

	col.xyz += ao * ambient;
	float spec = col.a - 0.05;
	//spec *= specular.a;
	col.xyz += spec * col.xyz;
	col.a = 1.0;

	gl_FragColor = col;
}
