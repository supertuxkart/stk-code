uniform sampler2D ntex;

uniform vec4 center[16];
uniform vec4 col[16];
uniform float energy[16];
uniform int lightcount;
uniform float spec;
uniform vec2 screen;
uniform mat4 invproj;
uniform mat4 viewm;

void main() {
	vec2 texc = gl_FragCoord.xy / screen;
	float z = texture2D(ntex, texc).a;

	vec4 xpos = 2.0 * vec4(texc, z, 1.0) - 1.0f;
	xpos = invproj * xpos;
	xpos /= xpos.w;

	vec3 diffuse = vec3(0.), specular = vec3(0.);

	for (int i = 0; i < lightcount; ++i) {
		vec4 pseudocenter = viewm * vec4(center[i].xyz, 1.0);
		pseudocenter /= pseudocenter.w;
		vec3 light_pos = pseudocenter.xyz;
		vec3 light_col = col[i].xyz;
		float d = distance(light_pos, xpos.xyz);
		float att = energy[i] * 200. / (4. * 3.14 * d * d);
		float spec_att = (energy[i] + 10) * 200. / (4. * 3.14 * d * d);

		vec3 norm = texture2D(ntex, texc).xyz;
		norm = (norm - 0.5) * 2.0;

		// Light Direction
		vec3 L = normalize(xpos.xyz - light_pos);

		float NdotL = max(0.0, dot(norm, -L));
		diffuse += NdotL * light_col * att;
		// Reflected light dir
		vec3 R = reflect(-L, norm);
		float RdotE = max(0.0, dot(R, normalize(xpos.xyz)));
		float Specular = pow(RdotE, spec);
		specular += Specular * light_col * spec_att;
	}

	gl_FragData[0] = vec4(diffuse, 1.);
	gl_FragData[1] = vec4(specular , 1.);
}
