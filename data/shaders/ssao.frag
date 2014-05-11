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
const float beta = 0.0001;
const float epsilon = .00001;
const float radius = .1;
const float k = 1.;

#define SAMPLES 16

const float invSamples = 1. / SAMPLES;

vec3 DecodeNormal(vec2 n);
vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

vec3 rand(vec2 co)
{
    float noiseX = clamp(fract(sin(dot(co ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
    float noiseY = clamp(fract(sin(dot(co ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
    return vec3(noiseX, noiseY, length(texture(noise_texture, co * pow(3.14159265359, 2.)).xyz));
}

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
    if (len < 0.2 || curdepth > 0.9955) discard;

    float randAngle = rand(uv).x;
    vec2 Xaxis = vec2(cos(randAngle), sin(randAngle));
    Xaxis = normalize(Xaxis);
    vec2 Yaxis = vec2(sin(randAngle), -cos(randAngle));
    Yaxis = normalize(Yaxis);

    float bl = 0.0;

    for(int i = 0; i < SAMPLES; ++i) {
        vec2 occluder_uv = samplePoints[i].x * Xaxis + samplePoints[i].y * Yaxis;
        occluder_uv *= samplePoints[i].w * radius;
        occluder_uv += uv;

        if (occluder_uv.x < 0. || occluder_uv.x > 1. || occluder_uv.y < 0. || occluder_uv.y > 1.) continue;

        float occluderFragmentDepth = texture(dtex, occluder_uv).x;
        vec4 OccluderPos = getPosFromUVDepth(vec3(occluder_uv, occluderFragmentDepth), InverseProjectionMatrix);

        vec3 vi = (OccluderPos - FragPos).xyz;
        bl += max(0, dot(vi, norm) - FragPos.z * beta) / (dot(vi, vi) + epsilon);
    }

    AO = max(pow(1.0 - 2. * sigma * bl * invSamples, k), 0.);
}
