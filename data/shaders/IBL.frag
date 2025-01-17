uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2D albedo;
uniform sampler2D ssao;
uniform sampler2D ctex;

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

vec3 gtaoMultiBounce(float visibility, vec3 albedo)
{
    // Jimenez et al. 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion"
    vec3 a =  2.0404 * albedo - 0.3324;
    vec3 b = -4.7951 * albedo + 0.6417;
    vec3 c =  2.7552 * albedo + 0.6903;

    return max(vec3(visibility), ((visibility * a + b) * visibility + c) * visibility);
}

// Main ===================================================================

void main(void)
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    vec3 normal = DecodeNormal(texture(ntex, uv).xy);

    float z = texture(dtex, uv).x;

    vec4 xpos = getPosFromUVDepth(vec3(uv, z), u_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos.xyz);
    // Extract roughness
    float specval = texture(ntex, uv).z;

    float ao = texture(ssao, uv).x;
    // Lagarde and de Rousiers 2014, "Moving Frostbite to PBR"
    float ao_spec = clamp(pow(max(dot(normal, eyedir), 0.) + ao, exp2(-16.0 * (1.0 - specval) - 1.0)) - 1.0 + ao, 0.0, 1.0);

#ifdef GL_ES
    Diff = vec4(0.25 * DiffuseIBL(normal) * ao, 1.);
    Spec = vec4(.25 * SpecularIBL(normal, eyedir, specval) * ao_spec, 1.);
#else
    vec3 surface_color = texture(ctex, uv).xyz;
    vec3 ao_multi = gtaoMultiBounce(ao, surface_color);
    vec3 ao_spec_multi = gtaoMultiBounce(ao_spec, surface_color);

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

    Diff = vec4(0.25 * DiffuseIBL(normal) * ao_multi, 1.);
    Spec = vec4(outColor.rgb * ao_spec_multi, 1.0);
#endif

}
