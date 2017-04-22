// From http://graphics.cs.aueb.gr/graphics/research_illumination.html
// "Real-Time Diffuse Global Illumination Using Radiance Hints"
// paper and shader code

uniform sampler2D ntex;
uniform sampler2D dtex;

uniform sampler3D SHR;
uniform sampler3D SHG;
uniform sampler3D SHB;

uniform vec3 extents;
uniform mat4 RHMatrix;
uniform mat4 InvRHMatrix;

vec4 SHBasis (const in vec3 dir)
{
    float   L00  = 0.282095;
    float   L1_1 = 0.488603 * dir.y;
    float   L10  = 0.488603 * dir.z;
    float   L11  = 0.488603 * dir.x;
    return vec4 (L11, L1_1, L10, L00);
}

vec3 SH2RGB (in vec4 sh_r, in vec4 sh_g, in vec4 sh_b, in vec3 dir)
{
    vec4 Y = vec4(1.023326*dir.x, 1.023326*dir.y, 1.023326*dir.z, 0.886226);
    return vec3 (dot(Y,sh_r), dot(Y,sh_g), dot(Y,sh_b));
}

out vec4 Diffuse;

#stk_include "utils/decodeNormal.frag"
#stk_include "utils/getPosFromUVDepth.frag"

vec3 resolution = vec3(32, 16, 32);

void main()
{
    vec2 uv = gl_FragCoord.xy / screen;
    vec3 GI = vec3(0.);

    float depth = texture2D(dtex, uv).x;
    // Discard background fragments
    if (depth==1.0) discard;

    vec4 pos_screen_space = getPosFromUVDepth(vec3(uv, depth), InverseProjectionMatrix);
    vec4 tmp = (InvRHMatrix * InverseViewMatrix * pos_screen_space);
    vec3 pos = tmp.xyz / tmp.w;
    vec3 normal_screen_space = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));
    vec3 normal = (transpose(ViewMatrix) * vec4(normal_screen_space, 0.)).xyz;

    // Convert to grid coordinates
    vec3 uvw = .5 + 0.5 * pos / extents;
    if (uvw.x < 0. || uvw.x > 1. || uvw.y < 0. || uvw.y > 1. || uvw.z < 0. || uvw.z > 1.) discard;

    // Sample the RH volume at 4 locations, one directly above the shaded point,
    // three on a ring 80degs away from the normal direction.
    vec3 rnd = vec3(0,0,0);

    // Generate the sample locations
    vec3 v_rand = vec3(0.5);
    vec3 tangent = normalize(cross(normal, v_rand));
    vec3 bitangent = cross(normal, tangent);
    vec3 D[4];
    D[0] = vec3(1.0,0.0,0.0);
    int i;
    for (i=1; i<4; i++)
    {
        D[i] = vec3(0.1, 0.8*cos((rnd.x*1.5+i)*6.2832/3.0), 0.8*sin((rnd.x*1.5+i)*6.2832/3.0));
        D[i] = normalize(D[i]);
    }

    for (i=0; i<4; i++)
    {
        vec3 SampleDir = normal * D[i].x + tangent * D[i].y + bitangent *D[i].z;
        vec3 SampleOffset = (0.5 * normal + SampleDir) / resolution;
        vec3 uvw_new = uvw + SampleOffset;

        vec4 IncidentSHR = texture(SHR, uvw_new);
        vec4 IncidentSHG = texture(SHG, uvw_new);
        vec4 IncidentSHB = texture(SHB, uvw_new);

        GI += SH2RGB(IncidentSHR, IncidentSHG, IncidentSHB, -normal);
    }
    GI /= 4;

    Diffuse = max(16. * vec4(GI, 1.), vec4(0.));
}
