uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2DArrayShadow shadowtex;
//uniform sampler2D warpx;
///uniform sampler2D warpy;

uniform vec3 direction;
uniform vec3 col;
//uniform int hasclouds;
//uniform vec2 wind;
//uniform float shadowoffset;

#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
uniform mat4 ShadowViewProjMatrixes[4];
#else
layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
};
#endif

in vec2 uv;
out vec4 Diff;
out vec4 Spec;

vec3 DecodeNormal(vec2 n);
vec3 getSpecular(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness);
vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

float getShadowFactor(vec3 pos, float bias, int index)
{
  //float a[5] = float[](3.4, 4.2, 5.0, 5.2, 1.1);
  
	vec2 shadowoffset[4] = vec2[](
		vec2(-1., -1.),
		vec2(-1., 1.),
		vec2(1., -1.),
		vec2(1., 1.)
	);

	vec4 shadowcoord = (ShadowViewProjMatrixes[index] * InverseViewMatrix * vec4(pos, 1.0));
	shadowcoord /= shadowcoord.w;
	vec2 shadowtexcoord = shadowcoord.xy * 0.5 + 0.5;
//	shadowcoord = (shadowcoord * 0.5) + vec3(0.5);

//	float movex = decdepth(texture(warpx, shadowcoord.xy));
//	float movey = decdepth(texture(warpy, shadowcoord.xy));
//	float dx = movex * 2.0 - 1.0;
//	float dy = movey * 2.0 - 1.0;
//	shadowcoord.xy += vec2(dx, dy);*/

//float shadowmapz = 2. * texture(shadowtex, vec3(shadowtexcoord, shadowcoord.z).x - 1.;
//	bias += smoothstep(0.001, 0.1, moved) * 0.014; // According to the warping
	float sum = 0.;
	for (int i = 0; i < 4; i++)
	{
		sum += texture(shadowtex, vec4(shadowtexcoord + shadowoffset[i] / 2048., float(index), 0.5 * shadowcoord.z + 0.5));
	}
	return sum / 4.;
}

void main() {
	float z = texture(dtex, uv).x;
	vec4 xpos = getPosFromUVDepth(vec3(uv, z), InverseProjectionMatrix);

	vec3 norm = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));
    float roughness =texture(ntex, uv).z;
    vec3 eyedir = -normalize(xpos.xyz);

	// Normalized on the cpu
    vec3 L = direction;

    float NdotL = max(0., dot(norm, L));

    vec3 Specular = getSpecular(norm, eyedir, L, col, roughness) * NdotL;


	vec3 outcol = NdotL * col;

//	if (hasclouds == 1)
//	{
//		vec2 cloudcoord = (xpos.xz * 0.00833333) + wind;
//		float cloud = texture(cloudtex, cloudcoord).x;
//		//float cloud = step(0.5, cloudcoord.x) * step(0.5, cloudcoord.y);

//		outcol *= cloud;
//	}

	// Shadows
	float bias = 0.005 * tan(acos(NdotL)); // According to the slope
	bias = clamp(bias, 0., 0.01);
	float factor;
	if (xpos.z < 5.)
		factor = getShadowFactor(xpos.xyz, bias, 0);
	else if (xpos.z < 6.)
	{
		float a = getShadowFactor(xpos.xyz, bias, 0), b = getShadowFactor(xpos.xyz, bias, 1);
		factor = mix(a, b, (xpos.z - 5.));
	}
	else if (xpos.z < 20.)
		factor = getShadowFactor(xpos.xyz, bias, 1);
	else if (xpos.z < 21.)
	{
		float a = getShadowFactor(xpos.xyz, bias, 1), b = getShadowFactor(xpos.xyz, bias, 2);
		factor = mix(a, b, (xpos.z - 20.));
	}
	else if (xpos.z < 50.)
		factor = getShadowFactor(xpos.xyz, bias, 2);
	else if (xpos.z < 55.)
	{
		float a = getShadowFactor(xpos.xyz, bias, 2), b = getShadowFactor(xpos.xyz, bias, 3);
		factor = mix(a, b, (xpos.z - 50.) / 5.);
	}
	else if (xpos.z < 145.)
		factor = getShadowFactor(xpos.xyz, bias, 3);
	else if (xpos.z < 150.)
	{
		factor = mix(getShadowFactor(xpos.xyz, bias, 3), 1., (xpos.z - 145.) / 5.);
	}
	else
		factor = 1.;
	Diff = vec4(factor * NdotL * col, 1.);
	Spec = vec4(factor * Specular, 1.);
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
