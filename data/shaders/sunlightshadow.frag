#version 130
uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2DShadow shadowtex;
//uniform sampler2D warpx;
///uniform sampler2D warpy;

uniform vec3 direction;
uniform vec3 col;
uniform vec2 screen;
uniform mat4 invproj;
uniform mat4 shadowmat;
//uniform int hasclouds;
//uniform vec2 wind;
//uniform float shadowoffset;

in vec2 uv;
out vec4 Diff;
out vec4 Spec;
out vec4 SpecularMap;

vec3 DecodeNormal(vec2 n)
{
  float z = dot(n, n) * 2. - 1.;
  vec2 xy = normalize(n) * sqrt(1. - z * z);
  return vec3(xy,z);
}

void main() {
	float z = texture(dtex, uv).x;
	vec4 xpos = 2.0 * vec4(uv, z, 1.0) - 1.0;
	xpos = invproj * xpos;
	xpos.xyz /= xpos.w;

	if (z < 0.03)
	{
		// Skyboxes are fully lit
		Diff = vec4(1.0);
		Spec = vec4(1.0);
		return;
	}

	vec3 norm = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));

	// Normalized on the cpu
	vec3 L = direction;

	float NdotL = max(0.0, dot(norm, L));
	vec3 R = reflect(L, norm);
	float RdotE = max(0.0, dot(R, normalize(xpos.xyz)));
	float Specular = pow(RdotE, 200);

	vec3 outcol = NdotL * col;

//	if (hasclouds == 1)
//	{
//		vec2 cloudcoord = (xpos.xz * 0.00833333) + wind;
//		float cloud = texture(cloudtex, cloudcoord).x;
//		//float cloud = step(0.5, cloudcoord.x) * step(0.5, cloudcoord.y);

//		outcol *= cloud;
//	}

	// Shadows
	vec4 shadowcoord = (shadowmat * vec4(xpos.xyz, 1.0));
	shadowcoord /= shadowcoord.w;
	vec2 shadowtexcoord = shadowcoord.xy * 0.5 + 0.5;
//	shadowcoord = (shadowcoord * 0.5) + vec3(0.5);

//	float movex = decdepth(texture(warpx, shadowcoord.xy));
//	float movey = decdepth(texture(warpy, shadowcoord.xy));
//	float dx = movex * 2.0 - 1.0;
//	float dy = movey * 2.0 - 1.0;
//	shadowcoord.xy += vec2(dx, dy);*/

	//float shadowmapz = 2. * texture(shadowtex, vec3(shadowtexcoord, shadowcoord.z).x - 1.;
	float bias = 0.002 * tan(acos(NdotL)); // According to the slope
	//	bias += smoothstep(0.001, 0.1, moved) * 0.014; // According to the warping
	bias = clamp(bias, 0.001, 0.014);
	float factor = texture(shadowtex, vec3(shadowtexcoord, 0.5 * (shadowcoord.z + bias) + 0.5));

	Diff = vec4(factor * NdotL * col, 1.);
	Spec = vec4(factor * Specular * col, 1.);
	SpecularMap = vec4(1.0);
	return;

//	float moved = (abs(dx) + abs(dy)) * 0.5;

//  float avi = 0.002;
//  float abi = 0.0025;

/*  float avi = 0.0018;
  float abi = 0.002;

	float bias = avi * tan(acos(NdotL)); // According to the slope
	bias += smoothstep(0.001, 0.1, moved) * abi; // According to the warping
	bias = clamp(bias, 0.001, abi);

	// This ID, and four IDs around this must match for a shadow pixel
	float right = texture(shadowtex, shadowcoord.xy + vec2(shadowoffset, 0.0)).a;
	float left = texture(shadowtex, shadowcoord.xy + vec2(-shadowoffset, 0.0)).a;
	float up = texture(shadowtex, shadowcoord.xy + vec2(0.0, shadowoffset)).a;
	float down = texture(shadowtex, shadowcoord.xy + vec2(0.0, -shadowoffset)).a;

	float matching = ((right + left + up + down) * 0.25) - shadowread.a;
	matching = abs(matching) * 400.0;

	// If the ID is different, we're likely in shadow - cut the bias to cut peter panning
	float off = 7.0 - step(abs(shadowread.a - depthread.a) - matching, 0.004) * 6.0;
	bias /= off;

	const float softness = 8.0; // How soft is the light?
	float shadowed = step(shadowmapz + bias, shadowcoord.z);
	float dist = (shadowcoord.z / shadowmapz) - 1.0;
	float penumbra = dist * softness / gl_FragCoord.z;
	penumbra *= shadowed;*/

/*	outcol.r = (shadowcoord.z - shadowmapz) * 50.0;
	outcol.g = moved;*/

//	FragColor = vec4(outcol, 0.05);
//	OtherOutput = vec4(shadowed, penumbra, shadowed, shadowed);
}
