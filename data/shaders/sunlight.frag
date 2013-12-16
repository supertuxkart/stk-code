uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2D cloudtex;

uniform vec3 center;
uniform vec3 col;
uniform vec2 screen;
uniform mat4 invprojview;
uniform int hasclouds;
uniform vec2 wind;

float decdepth(vec4 rgba) {
	return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
}

void main() {

	vec2 texc = gl_FragCoord.xy / screen;
	float z = decdepth(vec4(texture2D(dtex, texc).xyz, 0.0));

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

	vec3 outcol = NdotL * col;

	if (hasclouds == 1)
	{
		vec3 tmp = vec3(texc, z);
		tmp = tmp * 2.0 - 1.0;

		vec4 xpos = vec4(tmp, 1.0);
		xpos = invprojview * xpos;
		xpos.xyz /= xpos.w;

		vec2 cloudcoord = (xpos.xz * 0.00833333) + wind;
		float cloud = texture2D(cloudtex, cloudcoord).x;
		//float cloud = step(0.5, cloudcoord.x) * step(0.5, cloudcoord.y);

		outcol *= cloud;
	}

	gl_FragData[0] = vec4(outcol, 0.05);
	gl_FragData[1] = vec4(1.0);
}
