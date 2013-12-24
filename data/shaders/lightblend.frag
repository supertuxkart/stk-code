uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D ambient_occlusion;
uniform vec3 ambient;

void main()
{
	vec2 texc = gl_TexCoord[0].xy;

	vec3 diffuse = texture2D(diffuse, texc).xyz;
	vec3 spec = texture2D(specular, texc).xyz;
	float ao = texture2D(ambient_occlusion, texc).x;

	gl_FragColor = vec4(diffuse + spec + ao * ambient, 1.0);
}
