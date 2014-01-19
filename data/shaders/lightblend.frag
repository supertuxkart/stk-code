#version 130
uniform sampler2D diffuse;
uniform sampler2D specular;
uniform sampler2D ambient_occlusion;
uniform sampler2D specular_map;
uniform vec3 ambient;

in vec2 uv;

void main()
{
	vec2 texc = uv;

	vec3 diffuse = texture(diffuse, texc).xyz;
	vec3 spec = texture(specular, texc).xyz;
	float specmap = texture(specular_map, texc).x;
	float ao = texture(ambient_occlusion, texc).x;

	gl_FragColor = vec4(diffuse + spec * specmap + ao * ambient, 1.0);
}
