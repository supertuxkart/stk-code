uniform sampler2D diffuse_and_spec;
uniform sampler2D ambient_occlusion;
uniform vec3 ambient;
//uniform sampler2D spectex;

void main()
{
	vec2 texc = gl_TexCoord[0].xy;

	vec3 diffuse = texture2D(diffuse_and_spec, texc).xyz;
	vec3 specular = texture2D(diffuse_and_spec, texc).www;
	float ao = texture2D(ambient_occlusion, texc).x;

	gl_FragColor = vec4(diffuse + specular + ao * ambient, 1.0);
}
