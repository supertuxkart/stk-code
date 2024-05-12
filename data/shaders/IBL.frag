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

vec3 CalcViewPositionFromDepth(in vec2 uv)
{
    // Combine UV & depth into XY & Z (NDC)
    float z = texture(dtex, uv).x;
    return getPosFromUVDepth(vec3(uv, z), u_inverse_projection_matrix).xyz;
}

vec2 CalcCoordFromPosition(in vec3 pos)
{
    vec4 projectedCoord     = u_projection_matrix * vec4(pos, 1.0);
    projectedCoord.xy      /= projectedCoord.w;
    projectedCoord.xy       = projectedCoord.xy * 0.5 + 0.5;
    return projectedCoord.xy;
}

// Fade out edges of screen buffer tex
// 1 means full render tex, 0 means full IBL tex
float GetEdgeFade(vec2 coords)
{
    float gradL = smoothstep(0.0, 0.4, coords.x);
    float gradR = 1.0 - smoothstep(0.6, 1.0, coords.x);
    float gradT = smoothstep(0.0, 0.4, coords.y);
    float gradB = 1.0 - smoothstep(0.6, 1.0, coords.y);
    return min(min(gradL, gradR), min(gradT, gradB));
}

vec2 RayCast(vec3 dir, vec3 hitCoord)
{
    vec2 projectedCoord;
    vec3 dirstep = dir * 0.5f;
    float depth;
    hitCoord += dirstep;

    for (int i = 1; i <= 32; i++)
    {
        projectedCoord          = CalcCoordFromPosition(hitCoord);

        float depth             = CalcViewPositionFromDepth(projectedCoord).z;

        float directionSign = sign(abs(hitCoord.z) - depth);
        dirstep = dirstep * (1.0 - 0.5 * max(directionSign, 0.0));
        hitCoord += dirstep * (-directionSign);
    }

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

// Main ===================================================================

void main(void)
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    vec3 normal = DecodeNormal(texture(ntex, uv).xy);

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

    // Output color
    vec3 outColor;

    // Fallback (if the ray can't find an intersection we display the sky)
    vec3 fallback = .25 * SpecularIBL(normal, eyedir, specval);

    // Only calculate reflections if the reflectivity value is high enough,
    // otherwise just use specular IBL
    if (specval > 0.5)
    {
        // Reflection vector
        vec3 reflected              = reflect(-eyedir, normal);

        vec2 coords = RayCast(reflected, xpos.xyz);

        if (coords.x == 0.0 && coords.y == 0.0) {
            outColor = fallback;
        } else {
            // FIXME We need to generate mipmap to take into account the gloss map
            outColor = textureLod(albedo, coords, 0.f).rgb;
            outColor = mix(fallback, outColor, GetEdgeFade(coords));
            // TODO temporary measure the lack of mipmapping for RTT albedo
            // Implement it in proper way
            // Use (specval - 0.5) * 2.0 to bring specval from 0.5-1.0 range to 0.0-1.0 range
            outColor = mix(fallback, outColor, (specval - 0.5) * 2.0);
        }
    }
    else
    {
        outColor = fallback;
    }

    Spec = vec4(outColor.rgb, 1.0);
#endif

}
