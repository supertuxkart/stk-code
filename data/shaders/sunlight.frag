uniform sampler2D ntex;
uniform sampler2D cloudtex;

uniform vec3 center;
uniform vec3 col;
uniform vec2 screen;
uniform mat4 invproj;
uniform int hasclouds;
uniform vec2 wind;

void main() {

	vec2 texc = gl_FragCoord.xy / screen;
	float z = texture2D(ntex, texc).a;
	vec4 xpos = 2.0 * vec4(texc, z, 1.0) - 1.0;
	xpos = invproj * xpos;
	xpos.xyz /= xpos.w;

	if (z < 0.03)
	{
		// Skyboxes are fully lit
		gl_FragData[0] = vec4(1.0);
		gl_FragData[1] = vec4(1.0);
		return;
	}

	vec3 norm = texture2D(ntex, texc).xyz;
	norm = (norm - 0.5) * 2.0;

	// Normalized on the cpu
	vec3 L = center;

	float NdotL = max(0.0, dot(norm, L));
	vec3 R = reflect(L, norm);
	float RdotE = max(0.0, dot(R, normalize(xpos.xyz)));
	float Specular = pow(RdotE, 200);

	vec3 outcol = NdotL * col;

	if (hasclouds == 1)
	{
		vec2 cloudcoord = (xpos.xz * 0.00833333) + wind;
		float cloud = texture2D(cloudtex, cloudcoord).x;
		//float cloud = step(0.5, cloudcoord.x) * step(0.5, cloudcoord.y);

		outcol *= cloud;
	}

	gl_FragData[0] = vec4(NdotL * col, Specular+ 0.001); // Irrlicht force alpha test, can't be 0
	gl_FragData[1] = vec4(1.0);
}
