// From paper http://graphics.cs.williams.edu/papers/AlchemyHPG11/
// and improvements here http://graphics.cs.williams.edu/papers/SAOHPG12/

uniform sampler2D dtex;
uniform vec4 samplePoints[16];

#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
uniform vec2 screen;
#else
layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};
#endif

in vec2 uv;
out float AO;

const float sigma = 1.;
const float tau = 7.;
const float beta = 0.001;
const float epsilon = .00001;
const float radius = 1.;
const float k = 1.5;

#define SAMPLES 16

const float invSamples = 1. / SAMPLES;

vec3 getXcYcZc(int x, int y, float zC)
{
    // We use perspective symetric projection matrix hence P(0,2) = P(1, 2) = 0
    float xC= (1. - 2 * (float(x) + 0.5) / screen.x) * zC / ProjectionMatrix[0][0];
    float yC= (1. + 2 * (float(y) + 0.5) / screen.y) * zC / ProjectionMatrix[1][1];
    return vec3(xC, yC, zC);
}

void main(void)
{
    float lineardepth = textureLod(dtex, uv, 0.).x;
    int x = int(gl_FragCoord.x), y = int(gl_FragCoord.y);
    vec3 FragPos = getXcYcZc(x, y, lineardepth);

    // get the normal of current fragment
    vec3 ddx = dFdx(FragPos);
    vec3 ddy = dFdy(FragPos);
    vec3 norm = -normalize(cross(ddy, ddx));

    float r = radius / FragPos.z;
    float phi = 30. * (x ^ y) + 10. * x * y;
    float bl = 0.0;

    for(int i = 0; i < SAMPLES; ++i) {
        float alpha = (i + .5) * invSamples;
        float theta = 2. * 3.14 * tau * alpha + phi;
        float h = r * alpha;
        vec2 offset = h * vec2(cos(theta), sin(theta)) * screen;

        float m = round(log2(h) + 6);
        ivec2 ioccluder_uv = ivec2(x, y) + ivec2(offset);

        if (ioccluder_uv.x < 0 || ioccluder_uv.x > screen.x || ioccluder_uv.y < 0 || ioccluder_uv.y > screen.y) continue;

        float LinearoccluderFragmentDepth = textureLod(dtex, vec2(ioccluder_uv) / screen, m).x;
        vec3 OccluderPos = getXcYcZc(ioccluder_uv.x, ioccluder_uv.y, LinearoccluderFragmentDepth);

        vec3 vi = OccluderPos - FragPos;
        bl += max(0, dot(vi, norm) - FragPos.z * beta) / (dot(vi, vi) + epsilon);
    }

    AO = max(pow(1.0 - 2. * sigma * bl * invSamples, k), 0.);
}