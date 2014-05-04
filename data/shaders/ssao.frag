uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2D noise_texture;
uniform vec4 samplePoints[16];

#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
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
out float AO;

const float strengh = 5.;
const float radius = 1.f;

#define SAMPLES 16

const float invSamples = strengh / SAMPLES;

vec3 rand(vec2 co)
{
	float noiseX = clamp(fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
	float noiseY = clamp(fract(sin(dot(co ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
   return vec3(noiseX, noiseY, length(texture(noise_texture, co * pow(3.14159265359, 2.)).xyz));
}

vec3 DecodeNormal(vec2 n);
vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

void main(void)
{
	vec4 cur = texture(ntex, uv);
	float curdepth = texture(dtex, uv).x;
    vec4 FragPos = getPosFromUVDepth(vec3(uv, curdepth), InverseProjectionMatrix);

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
		vec4 sampleProj = ProjectionMatrix * samplePos;
		sampleProj /= sampleProj.w;

		bool isInsideTexture = (sampleProj.x > -1.) && (sampleProj.x < 1.) && (sampleProj.y > -1.) && (sampleProj.y < 1.);
		// get the depth of the occluder fragment
		float occluderFragmentDepth = texture(dtex, (sampleProj.xy * 0.5) + 0.5).x;
		// Position of the occluder fragment in worldSpace
		vec4 occluderPos = getPosFromUVDepth(vec3((sampleProj.xy * 0.5) + 0.5, occluderFragmentDepth), InverseProjectionMatrix);

		bool isOccluded = isInsideTexture && (sampleProj.z > (2. * occluderFragmentDepth - 1.0)) && (distance(FragPos, occluderPos) < radius);
		bl += isOccluded ? samplePoints[i].z * smoothstep(5 * radius, 0, distance(samplePos, FragPos)) : 0.;
	}

	AO = max(1.0 - bl * invSamples, 0.);
}
