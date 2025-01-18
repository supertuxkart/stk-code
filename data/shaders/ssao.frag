// From paper http://graphics.cs.williams.edu/papers/AlchemyHPG11/
// and improvements here http://graphics.cs.williams.edu/papers/SAOHPG12/
// and implementations here https://github.com/google/filament/blob/026b985c07b7eec4f678e0e5130d0a4e742e9c61/filament/src/materials/ssao/saoImpl.fs

uniform sampler2D dtex;
uniform float radius;
uniform float k;
uniform float sigma;
out float AO;

const float thickness = 10.0;

#define SAMPLES 4
const float invSamples = 0.25; // 1. / SAMPLES

vec3 getXcYcZc(int x, int y, float zC)
{
    // We use perspective symetric projection matrix hence P(0,2) = P(1, 2) = 0
    float xC= (2. * (float(x)) / u_screen.x - 1.) * zC / u_projection_matrix[0][0];
    float yC= (2. * (float(y)) / u_screen.y - 1.) * zC / u_projection_matrix[1][1];
    return vec3(xC, yC, zC);
}

float interleavedGradientNoise(highp vec2 w)
{
    const vec3 m = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(m.z * fract(dot(w, m.xy)));
}

void main(void)
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    float lineardepth = textureLod(dtex, uv, 0.).x;
    highp int x = int(gl_FragCoord.x), y = int(gl_FragCoord.y);
    vec3 FragPos = getXcYcZc(x, y, lineardepth);

    // get the normal of current fragment
    vec3 ddx = dFdx(FragPos);
    vec3 ddy = dFdy(FragPos);
    vec3 norm = normalize(cross(ddy, ddx));

    float r = radius / FragPos.z;
    float phi = interleavedGradientNoise(vec2(gl_FragCoord.x, gl_FragCoord.y));
    float bl = 0.0;
    float m = log2(r) + 6. + log2(invSamples);

    float peak = 0.1 * radius;
    float peak2 = peak * peak;
    float intensity = 2.0 * 3.14159 * sigma * peak * invSamples;

    // Apply stronger bias when the resolution is lower.
    float horizon = min(100. / min(u_screen.x, u_screen.y), 0.3);
    float bias =  min(1. / min(u_screen.x, u_screen.y), 0.003);

    float theta = phi * 2.0 * 2.4 * 3.14159;
    vec2 rotations = vec2(cos(theta), sin(theta)) * u_screen;
    vec2 offset = vec2(cos(invSamples), sin(invSamples));

    for(int i = 0; i < SAMPLES; ++i)
    {
        float alpha = (float(i) + .5) * invSamples;
        rotations = vec2(rotations.x * offset.x - rotations.y * offset.y, rotations.x * offset.y + rotations.y * offset.x);
        float h = r * alpha;
        vec2 localoffset = h * rotations;

        ivec2 ioccluder_uv = clamp(ivec2(x, y) + ivec2(localoffset), ivec2(0), ivec2(u_screen));

        float LinearoccluderFragmentDepth = textureLod(dtex, vec2(ioccluder_uv) / u_screen, max(m, 0.)).x;
        vec3 OccluderPos = getXcYcZc(ioccluder_uv.x, ioccluder_uv.y, LinearoccluderFragmentDepth);

        vec3 vi = OccluderPos - FragPos;
        float vv = dot(vi, vi);
        float vn = dot(vi, norm);
        float w = max(0.0, 1.0 - vv / thickness / thickness);
        w = w * w;
        w *= step(vv * horizon * horizon, vn * vn);
        bl += w * max(0., vn - FragPos.z * bias) / (vv + peak);
    }

    AO = pow(max(1.0 - sqrt(bl * intensity), 0.), k);
}
