uniform sampler2D ntex;

uniform vec3 center;
uniform vec3 col;
uniform float r;
uniform float spec;
uniform vec2 screen;
uniform mat4 invproj;

void main() {
	vec2 texc = gl_FragCoord.xy / screen;
	float z = texture2D(ntex, texc).a;

	vec4 xpos = 2.0 * vec4(texc, z, 1.0) - 1.0f;
	xpos = invproj * xpos;
	xpos /= xpos.w;

	float d = distance(center, xpos.xyz);
	if (d > r) discard;
	float att = 200.0 / (4. * 3.14 * d * d);

	vec3 norm = texture2D(ntex, texc).xyz;
	norm = (norm - 0.5) * 2.0;

	// Light Direction
	vec3 L = normalize(xpos.xyz - center);

	float NdotL = max(0.0, dot(norm, -L));
	// Reflected light dir
	vec3 R = reflect(-L, norm);
	float RdotE = max(0.0, dot(R, normalize(xpos.xyz)));
	float Specular = pow(RdotE, spec);

	gl_FragData[0] = vec4(NdotL * col * att, 1.);
	gl_FragData[1] = vec4(Specular * col, 1.);
}
