uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2D noise_texture;
uniform mat4 invprojm;
uniform mat4 projm;
uniform vec4 samplePoints[16];

#if __VERSION__ >= 130
in vec2 uv;
out float AO;
#else
varying vec2 uv;
#define AO gl_FragColor.x
#endif

const float strengh = 4.;
const float radius = .4f;

#define SAMPLES 16

const float invSamples = strengh / SAMPLES;

vec3 rand(vec2 co)
{
   return texture(noise_texture, co*20.16).xyz;
}

vec3 DecodeNormal(vec2 n)
{
  float z = dot(n, n) * 2. - 1.;
  vec2 xy = normalize(n) * sqrt(1. - z * z);
  return vec3(xy,z);
}

void main(void)
{
	vec4 cur = texture(ntex, uv);
	float curdepth = texture(dtex, uv).x;
	vec4 FragPos = invprojm * (2.0f * vec4(uv, curdepth, 1.0f) - 1.0f);
	FragPos /= FragPos.w;

	// get the normal of current fragment
	vec3 norm = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));
	// Workaround for nvidia and skyboxes
	float len = dot(vec3(1.0), abs(cur.xyz));
	if (len < 0.2 || curdepth > 0.9955) discard;
	// Make a tangent as random as possible
	vec3 randvect = rand(uv);
	vec3 tangent = normalize(cross(norm, randvect));
	vec3 bitangent = cross(norm, tangent);

	float bl = 0.0;

	for(int i = 0; i < SAMPLES; ++i) {
		vec3 sampleDir = samplePoints[i].x * tangent + samplePoints[i].y * bitangent + samplePoints[i].z * norm;
		sampleDir *= samplePoints[i].w;
		vec4 samplePos = FragPos + radius * vec4(sampleDir, 0.0);
		vec4 sampleProj = projm * samplePos;
		sampleProj /= sampleProj.w;

		bool isInsideTexture = (sampleProj.x > -1.) && (sampleProj.x < 1.) && (sampleProj.y > -1.) && (sampleProj.y < 1.);
		// get the depth of the occluder fragment
		float occluderFragmentDepth = texture(dtex, (sampleProj.xy * 0.5) + 0.5).x;
		// Position of the occluder fragment in worldSpace
		vec4 occluderPos = invprojm * vec4(sampleProj.xy, 2.0 * occluderFragmentDepth - 1.0, 1.0f);
		occluderPos /= occluderPos.w;

		bool isOccluded = isInsideTexture && (sampleProj.z > (2. * occluderFragmentDepth - 1.0)) && (distance(FragPos, occluderPos) < radius);
		bl += isOccluded ? smoothstep(radius, 0, distance(samplePos, FragPos)) : 0.;
	}

	AO = 1.0 - bl * invSamples;
}
