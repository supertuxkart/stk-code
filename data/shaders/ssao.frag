#version 120

uniform sampler2D normals_and_depth;
uniform sampler2D depth;
uniform mat4 invprojm;
uniform mat4 projm;
uniform vec4 samplePoints[16];

const float strengh = 1.;
const float radius = 0.1f;
const float threshold = 0.1;

#define SAMPLES 16

const float invSamples = strengh / SAMPLES;

float decdepth(vec4 rgba) {
	return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
}

void main(void)
{
	// A set of Random(tm) vec2's. 8 1s, 6 0.7s, 2 0.4
	// Again not using const because of broken Intel Windows drivers


	vec2 uv = gl_TexCoord[0].xy;

	vec4 cur = texture2D(normals_and_depth, uv);
	float curdepth = decdepth(vec4(texture2D(depth, uv).xyz, 0.0));
	vec4 FragPos = invprojm * (2.0f * vec4(uv, curdepth, 1.0f) - 1.0f);
	FragPos /= FragPos.w;

	// get the normal of current fragment
	vec3 norm = normalize(cur.xyz * vec3(2.0) - vec3(1.0));
	if (curdepth > 0.99) discard;
	vec3 tangent = normalize(cross(norm, norm.yzx));
	vec3 bitangent = cross(norm, tangent);

	float bl = 0.0;

	for(int i = 0; i < SAMPLES; ++i) {
		vec3 sampleDir = samplePoints[i].x * tangent + samplePoints[i].y * bitangent + samplePoints[i].z * norm;
		vec4 samplePos = FragPos + radius * vec4(sampleDir, 0.0);
		vec4 sampleProj = projm * samplePos;
		sampleProj /= sampleProj.w;

		// get the depth of the occluder fragment
		float occluderFragmentDepth = decdepth(vec4(texture2D(depth, (sampleProj.xy * 0.5) + 0.5).xyz, 0.0));

		float depthDifference = sampleProj.z - (2. * occluderFragmentDepth - 1.0);
		bl += (abs(depthDifference) < threshold) ? step(0., depthDifference) : 0.0;
	}

	// output the result
	float ao = 1.0 - bl * invSamples;

	gl_FragColor = vec4(vec3(ao), curdepth + 0.05); // offset so that the alpha test doesn't kill us
}
