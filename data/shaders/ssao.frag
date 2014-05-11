// From paper http://graphics.cs.williams.edu/papers/AlchemyHPG11/
// and improvements here http://graphics.cs.williams.edu/papers/SAOHPG12/

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

const float sigma = 1.;
const float tau = 7.;
const float beta = 0.0001;
const float epsilon = .00001;
const float radius = 2.;
const float k = 1.5;

#define SAMPLES 16

const float invSamples = 1. / SAMPLES;

vec3 DecodeNormal(vec2 n);
vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

void main(void)
{
    vec4 cur = texture(ntex, uv);
    float curdepth = texture(dtex, uv).x;
    vec4 FragPos = getPosFromUVDepth(vec3(uv, curdepth), InverseProjectionMatrix);

    // get the normal of current fragment
    vec3 ddx = dFdx(FragPos.xyz);
    vec3 ddy = dFdy(FragPos.xyz);
    vec3 norm = normalize(cross(ddy, ddx));
    // Workaround for nvidia and skyboxes
    float len = dot(vec3(1.0), abs(cur.xyz));
    if (len < 0.2) discard;

    int x = int(1024. * uv.x), y = int(1024. * uv.y);
    float r = radius / FragPos.z;
    float phi = 30. * (x ^ y) + 10. * x * y;
    float bl = 0.0;

    for(int i = 0; i < SAMPLES; ++i) {
        float alpha = (i + .5) * invSamples;
        float theta = 2. * 3.14 * tau * alpha + phi;
        float h = r * alpha;
        vec2 occluder_uv = h * vec2(cos(theta), sin(theta));
        occluder_uv += uv;

        if (occluder_uv.x < 0. || occluder_uv.x > 1. || occluder_uv.y < 0. || occluder_uv.y > 1.) continue;

        float m = round(log2(h)) + 6.;

        float occluderFragmentDepth = textureLod(dtex, occluder_uv, m).x;
        vec4 OccluderPos = getPosFromUVDepth(vec3(occluder_uv, occluderFragmentDepth), InverseProjectionMatrix);

        vec3 vi = (OccluderPos - FragPos).xyz;
        bl += max(0, dot(vi, norm) - FragPos.z * beta) / (dot(vi, vi) + epsilon);
    }

    AO = max(pow(1.0 - 2. * sigma * bl * invSamples, k), 0.);
}
