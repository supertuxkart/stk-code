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
	float att = 1.0 - smoothstep(0.0, r, d);

	vec3 norm = texture2D(ntex, texc).xyz;
	norm = (norm - 0.5) * 2.0;

	// Light Direction
	vec3 L = normalize(xpos.xyz - center);
	vec3 eyedir = normalize(xpos.xyz);
	vec3 H = normalize(-L + eyedir);

	float NdotL = max(0.0, dot(norm, -L)) * att;
	float NdotH = max(0.0, dot(norm, H));
	NdotH = pow(NdotH, spec);
	NdotH += 0.05; // offset so that the alpha test doesn't kill us

	gl_FragColor = NdotL * vec4(NdotL * col, NdotH);
}
