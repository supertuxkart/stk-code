uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D ambient_occlusion;
uniform sampler2D specular_map;
uniform vec3 ambient;

void main()
{
	vec2 texc = gl_TexCoord[0].xy;

	vec3 diffuse = texture2D(diffuse, texc).xyz;
	vec3 spec = texture2D(specular, texc).xyz;
	float specmap = texture2D(specular_map, texc).x;
	float ao = texture2D(ambient_occlusion, texc).x;

	gl_FragColor = vec4(diffuse + spec * specmap + ao * ambient, 1.0);
}
