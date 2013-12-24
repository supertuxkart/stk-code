#version 120
uniform sampler2D normals_and_depth;
uniform mat4 invprojm;
uniform mat4 projm;
uniform vec4 samplePoints[16];

const float strengh = 4.;
const float radius = .1f;

#define SAMPLES 16

const float invSamples = strengh / SAMPLES;

void main(void)
{
	// A set of Random(tm) vec2's. 8 1s, 6 0.7s, 2 0.4
	// Again not using const because of broken Intel Windows drivers


	vec2 uv = gl_TexCoord[0].xy;

	vec4 cur = texture2D(normals_and_depth, uv);
	float curdepth = texture2D(normals_and_depth, uv).a;
	vec4 FragPos = invprojm * (2.0f * vec4(uv, curdepth, 1.0f) - 1.0f);
	FragPos /= FragPos.w;

	// get the normal of current fragment
	vec3 norm = normalize(cur.xyz * vec3(2.0) - vec3(1.0));
	// Workaround for nvidia and skyboxes
	float len = dot(vec3(1.0), abs(cur.xyz));
	if (len < 0.2 || curdepth > 0.999) discard;
	vec3 tangent = normalize(cross(norm, norm.yzx));
	vec3 bitangent = cross(norm, tangent);

	float bl = 0.0;

	for(int i = 0; i < SAMPLES; ++i) {
		vec3 sampleDir = samplePoints[i].x * tangent + samplePoints[i].y * bitangent + samplePoints[i].z * norm;
		sampleDir *= samplePoints[i].w;
		vec4 samplePos = FragPos + radius * vec4(sampleDir, 0.0);
		vec4 sampleProj = projm * samplePos;
		sampleProj /= sampleProj.w;
		// Projection of sampleDir over nom
		float cosTheta = samplePoints[i].z;

		bool isInsideTexture = (sampleProj.x > -1.) && (sampleProj.x < 1.) && (sampleProj.y > -1.) && (sampleProj.y < 1.);
		// get the depth of the occluder fragment
		float occluderFragmentDepth = texture2D(normals_and_depth, (sampleProj.xy * 0.5) + 0.5).a;
		// Position of the occluder fragment in worldSpace
		vec4 occluderPos = invprojm * vec4(sampleProj.xy, 2.0 * occluderFragmentDepth - 1.0, 1.0f);
		occluderPos /= occluderPos.w;

		bool isOccluded = isInsideTexture && (sampleProj.z > (2. * occluderFragmentDepth - 1.0)) && (distance(FragPos, occluderPos) < radius);
		bl += isOccluded ? smoothstep(radius, 0, distance(samplePos, FragPos)) * cosTheta : 0.;
	}

	// output the result
	float ao = 1.0 - bl * invSamples;

	gl_FragColor = vec4(vec3(ao), curdepth + 0.05); // offset so that the alpha test doesn't kill us
}
