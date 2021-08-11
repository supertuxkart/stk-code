uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2D albedo;

#ifdef GL_ES
layout (location = 0) out vec4 Diff;
layout (location = 1) out vec4 Spec;
#else
out vec4 Diff;
out vec4 Spec;
#endif

#stk_include "utils/decodeNormal.frag"
#stk_include "utils/getPosFromUVDepth.frag"
#stk_include "utils/DiffuseIBL.frag"
#stk_include "utils/SpecularIBL.frag"


float makeLinear(float f, float n, float z)
{
    return (2.0f * n) / (f + n - z * (f - n));
}

vec3 CalcViewPositionFromDepth(in vec2 TexCoord)
{
    // Combine UV & depth into XY & Z (NDC)
    float z = makeLinear(1000.0, 1.0, textureLod(dtex, TexCoord, 0.).x);
    vec3 rawPosition                = vec3(TexCoord, z);

    // Convert from (0, 1) range to (-1, 1)
    vec4 ScreenSpacePosition        = vec4( rawPosition * 2.0 - 1.0, 1.0);

    // Undo Perspective transformation to bring into view space
    vec4 ViewPosition               = u_inverse_projection_matrix * ScreenSpacePosition;

    // Perform perspective divide and return
    return                          ViewPosition.xyz / ViewPosition.w;
}

float GetVignette(vec2 coords, float factor)
{
    vec2 inside = coords - 0.5;
    float vignette = 1. - dot(inside, inside) * 4.0;
    return clamp(pow(vignette, factor), 0., 1.0);
}

vec2 RayCast(vec3 dir, inout vec3 hitCoord, out float dDepth)
{
    dir *= 0.25f;

    for(int i = 0; i < 8; ++i) {
        hitCoord               += dir;

        vec4 projectedCoord     = u_projection_matrix * vec4(hitCoord, 1.0);
        projectedCoord.xy      /= projectedCoord.w;
        projectedCoord.xy       = projectedCoord.xy * 0.5 + 0.5;

        float depth             = CalcViewPositionFromDepth(projectedCoord.xy).z;
        dDepth                  = hitCoord.z - depth;

        if (dDepth < 0.0)
        {
            if (projectedCoord.x > 0.0 && projectedCoord.x < 1.0 &&
                projectedCoord.y > 0.0 && projectedCoord.y < 1.0)
            {
                return projectedCoord.xy;
            }
            else
            {
                return vec2(0.f);
            }
        }
    }

    return vec2(0.f);
}

// Main ===================================================================

void main(void)
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    vec3 normal = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));

    Diff = vec4(0.25 * DiffuseIBL(normal), 1.);

    float z = texture(dtex, uv).x;

    vec4 xpos = getPosFromUVDepth(vec3(uv, z), u_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos.xyz);
    // Extract roughness
    float specval = texture(ntex, uv).z;

#ifdef GL_ES
    Spec = vec4(.25 * SpecularIBL(normal, eyedir, specval), 1.);
#else
    // :::::::: Compute Space Screen Reflection ::::::::::::::::::::::::::::::::::::

    // Fallback (if the ray can't find an intersection we display the sky)
    vec3 fallback = .25 * SpecularIBL(normal, eyedir, specval);

    vec3 View_Pos               = CalcViewPositionFromDepth(uv);

    // Reflection vector
    vec3 reflected              = normalize(reflect(eyedir, normal));

    // Ray cast
    vec3 hitPos                 = View_Pos.xyz;
    float dDepth;
    float minRayStep            = 50.0f;

    vec2 coords = RayCast(reflected * max(minRayStep, -View_Pos.z),
                            hitPos, dDepth);

    vec3 outColor;

    if (coords.x == 0.0 && coords.y == 0.0) {
        outColor = fallback;
    } else {
        // FIXME We need to generate mipmap to take into account the gloss map
        outColor = textureLod(albedo, coords, 0.f).rgb;
        outColor = mix(fallback, outColor, GetVignette(coords, 2.5));
    }
    
    // TODO temporary measure the lack of mipmaping for RTT albedo
    // Implement it in proper way
    outColor = mix(fallback, outColor, specval);
    Spec = vec4(outColor.rgb, 1.0);
#endif

}
