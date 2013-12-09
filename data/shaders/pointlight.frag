uniform sampler2D ntex;
uniform sampler2D dtex;

uniform vec3 center;
uniform vec3 col;
uniform vec3 campos;
uniform float r;
uniform float spec;
uniform vec2 screen;
uniform mat4 invprojview;

float decdepth(vec4 rgba) {
	return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
}

void main() {

	vec2 texc = gl_FragCoord.xy / screen;
	float z = decdepth(vec4(texture2D(dtex, texc).xyz, 0.0));

	if (z < 0.03) discard;

	vec3 tmp = vec3(texc, z);
	tmp = tmp * 2.0 - 1.0;

	vec4 xpos = vec4(tmp, 1.0);
	xpos = invprojview * xpos;
	xpos.xyz /= xpos.w;

	float d = distance(center, xpos.xyz);
	if (d > r) discard;

	float att = 1.0 - smoothstep(0.0, r, d);

	vec3 norm = texture2D(ntex, texc).xyz;
	norm = (norm - 0.5) * 2.0;

	vec3 camdir = normalize(campos - xpos.xyz);

	vec3 L = normalize(center - xpos.xyz);
	vec3 H = normalize(L + camdir);

	float NdotL = max(0.0, dot(norm, L)) * att;
	if (NdotL < 0.01) discard;
	float NdotH = max(0.0, dot(norm, H));
	NdotH = pow(NdotH, spec);
	NdotH += 0.05; // offset so that the alpha test doesn't kill us

	gl_FragColor = NdotL * vec4(col, NdotH);
}
