// From paper http://graphics.cs.williams.edu/papers/AlchemyHPG11/
// and improvements here http://graphics.cs.williams.edu/papers/SAOHPG12/

uniform sampler2D dtex;
uniform float radius;
uniform float k;
uniform float sigma;
out float AO;

const float tau = 7.;
const float beta = 0.002;
const float epsilon = .00001;

#define SAMPLES 16
const float invSamples = 0.0625; // 1. / SAMPLES

vec3 getXcYcZc(int x, int y, float zC)
{
    // We use perspective symetric projection matrix hence P(0,2) = P(1, 2) = 0
    float xC= (2. * (float(x)) / screen.x - 1.) * zC / ProjectionMatrix[0][0];
    float yC= (2. * (float(y)) / screen.y - 1.) * zC / ProjectionMatrix[1][1];
    return vec3(xC, yC, zC);
}

void main(void)
{
    vec2 uv = gl_FragCoord.xy / screen;
    float lineardepth = textureLod(dtex, uv, 0.).x;
    int x = int(gl_FragCoord.x), y = int(gl_FragCoord.y);
    vec3 FragPos = getXcYcZc(x, y, lineardepth);

    // get the normal of current fragment
    vec3 ddx = dFdx(FragPos);
    vec3 ddy = dFdy(FragPos);
    vec3 norm = normalize(cross(ddy, ddx));

    float r = radius / FragPos.z;
    float phi = 3. * float((x ^ y) + x * y);
    float bl = 0.0;
    float m = log2(r) + 6. + log2(invSamples);

    float theta = mod(2. * 3.14 * tau * .5 * invSamples + phi, 6.283185307179586);
    vec2 rotations = vec2(cos(theta), sin(theta)) * screen;
    vec2 offset = vec2(cos(invSamples), sin(invSamples));

    for(int i = 0; i < SAMPLES; ++i) {
        float alpha = (float(i) + .5) * invSamples;
        rotations = vec2(rotations.x * offset.x - rotations.y * offset.y, rotations.x * offset.y + rotations.y * offset.x);
        float h = r * alpha;
        vec2 localoffset = h * rotations;

        m = m + .5;
        ivec2 ioccluder_uv = ivec2(x, y) + ivec2(localoffset);

        if (ioccluder_uv.x < 0 || ioccluder_uv.x > int(screen.x) || ioccluder_uv.y < 0 || ioccluder_uv.y > int(screen.y)) continue;

        float LinearoccluderFragmentDepth = textureLod(dtex, vec2(ioccluder_uv) / screen, max(m, 0.)).x;
        vec3 OccluderPos = getXcYcZc(ioccluder_uv.x, ioccluder_uv.y, LinearoccluderFragmentDepth);

        vec3 vi = OccluderPos - FragPos;
        bl += max(0., dot(vi, norm) - FragPos.z * beta) / (dot(vi, vi) + epsilon);
    }

    AO = max(pow(1.0 - min(2. * sigma * bl * invSamples, 0.99), k), 0.);
}
