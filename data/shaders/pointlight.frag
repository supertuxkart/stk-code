uniform sampler2D ntex;
uniform sampler2D dtex;

uniform vec3 center;
uniform vec3 col;
uniform float r;
uniform float spec;
uniform vec2 screen;
uniform mat4 invproj;

float decdepth(vec4 rgba) {
	return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
}

void main() {
	vec2 texc = gl_FragCoord.xy / screen;
	float z = decdepth(vec4(texture2D(dtex, texc).xyz, 0.0));

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
