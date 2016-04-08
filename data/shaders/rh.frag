// From http://graphics.cs.aueb.gr/graphics/research_illumination.html
// "Real-Time Diffuse Global Illumination Using Radiance Hints"
// paper and shader code

uniform float R_wcs = 10.;            // Rmax: maximum sampling distance (in WCS units)
uniform vec3 extents;
uniform mat4 RHMatrix;
uniform mat4 RSMMatrix;
uniform sampler2D dtex;
uniform sampler2D ctex;
uniform sampler2D ntex;
uniform vec3 suncol;

flat in int slice;
layout (location = 0) out vec4 SHRed;
layout (location = 1) out vec4 SHGreen;
layout (location = 2) out vec4 SHBlue;

vec3 resolution = vec3(32, 16, 32);
#define SAMPLES 16

vec4 SHBasis (const in vec3 dir)
{
    float   L00  = 0.282095;
    float   L1_1 = 0.488603 * dir.y;
    float   L10  = 0.488603 * dir.z;
    float   L11  = 0.488603 * dir.x;
    return vec4 (L11, L1_1, L10, L00);
}

vec4 DirToSh(vec3 dir, float flux)
{
    return SHBasis (dir) * flux;
}

// We need to manually unroll the loop, otherwise Nvidia driver crashes.
void loop(in int i,
          in vec3 RHcenter,in vec3 RHCellSize, in vec2 RHuv, in float RHdepth,
          inout vec4 SHr, inout vec4 SHg, inout vec4 SHb)
{
    // produce a new sample location on the RSM texture
    float alpha = (i + .5) / SAMPLES;
    float theta = 2. * 3.14 * 7. * alpha;
    float h = alpha;
    vec2 offset = h * vec2(cos(theta), sin(theta));
    vec2 uv = RHuv + offset * 0.01;

    // Get world position and normal from the RSM sample
    float depth = texture(dtex, uv).x;
    vec4 RSMPos = inverse(RSMMatrix) * (2. * vec4(uv, depth, 1.) - 1.);
    RSMPos /= RSMPos.w;
    vec3 RSMAlbedo = texture(ctex, uv).xyz;
    vec3 normal = normalize(2. * texture(ntex, uv).xyz - 1.);

    // Sampled location inside the RH cell
    vec3 offset3d = vec3(uv, 0);
    vec3 SamplePos = RHcenter + .5 * offset3d.xzy * RHCellSize;

    // Normalize distance to RSM sample
    float dist = distance(SamplePos, RSMPos.xyz) / R_wcs;
    // Determine the incident direction.
    // Avoid very close samples (and numerical instability problems)
    vec3 RSM_to_RH_dir = (dist <= 0.1) ? vec3(0.) : normalize(SamplePos - RSMPos.xyz);
    float dotprod = max(dot(RSM_to_RH_dir, normal.xyz), 0.);
    float factor = dotprod / (0.1 + dist * dist);

    vec3 color = RSMAlbedo.rgb * factor * suncol.rgb;

    SHr += DirToSh(RSM_to_RH_dir, color.r);
    SHg += DirToSh(RSM_to_RH_dir, color.g);
    SHb += DirToSh(RSM_to_RH_dir, color.b);
}

void main(void)
{
    vec3 normalizedRHCenter = 2. * vec3(gl_FragCoord.xy, slice) / resolution - 1.;
    vec3 RHcenter = (RHMatrix * vec4(normalizedRHCenter * extents, 1.)).xyz;

    vec4 ShadowProjectedRH = RSMMatrix * vec4(RHcenter, 1.);

    vec3 RHCellSize = extents / resolution;
    vec2 RHuv = .5 * ShadowProjectedRH.xy / ShadowProjectedRH.w + .5;
    float RHdepth = .5 * ShadowProjectedRH.z / ShadowProjectedRH.w + .5;

    vec4  SHr = vec4(0.);
    vec4  SHg = vec4(0.);
    vec4  SHb = vec4(0.);

    int x = int(gl_FragCoord.x), y = int(gl_FragCoord.y);
    float phi = 30. * (x ^ y) + 10. * x * y;

    loop(0, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(1, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(2, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(3, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(4, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(5, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(6, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(7, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(8, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(9, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(10, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(11, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(12, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(13, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(14, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);
    loop(15, RHcenter, RHCellSize, RHuv, RHdepth, SHr, SHg, SHb);

    SHr /= 3.14159 * SAMPLES;
    SHg /= 3.14159 * SAMPLES;
    SHb /= 3.14159 * SAMPLES;

    SHRed = SHr;
    SHGreen = SHg;
    SHBlue = SHb;
}
