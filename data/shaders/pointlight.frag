#version 130
uniform sampler2D ntex;
uniform sampler2D dtex;

uniform vec4 center[16];
uniform vec4 col[16];
uniform float energy[16];
uniform float spec;
uniform mat4 invproj;
uniform mat4 viewm;

in vec2 uv;
out vec4 Diffuse;
out vec4 Specular;

vec3 DecodeNormal(vec2 n)
{
  float z = dot(n, n) * 2. - 1.;
  vec2 xy = normalize(n) * sqrt(1. - z * z);
  return vec3(xy,z);
}

void main() {
	vec2 texc = uv;
	float z = texture(dtex, texc).x;
	vec3 norm = normalize(DecodeNormal(2. * texture(ntex, texc).xy - 1.));

	vec4 xpos = 2.0 * vec4(texc, z, 1.0) - 1.0f;
	xpos = invproj * xpos;
	xpos /= xpos.w;
	vec3 eyedir = normalize(xpos.xyz);

	vec3 diffuse = vec3(0.), specular = vec3(0.);

	for (int i = 0; i < 16; ++i) {
		vec4 pseudocenter = viewm * vec4(center[i].xyz, 1.0);
		pseudocenter /= pseudocenter.w;
		vec3 light_pos = pseudocenter.xyz;
		vec3 light_col = col[i].xyz;
		float d = distance(light_pos, xpos.xyz);
		float att = energy[i] * 200. / (4. * 3.14 * d * d);
		float spec_att = (energy[i] + 10.) * 200. / (4. * 3.14 * d * d);

		// Light Direction
		vec3 L = normalize(xpos.xyz - light_pos);

		float NdotL = max(0.0, dot(norm, -L));
		diffuse += NdotL * light_col * att;
		// Reflected light dir
		vec3 R = reflect(-L, norm);
		float RdotE = max(0.0, dot(R, eyedir));
		float Specular = pow(RdotE, spec);
		specular += Specular * light_col * spec_att;
	}

	Diffuse = vec4(diffuse, 1.);
	Specular = vec4(specular , 1.);
}
