uniform sampler2D tex;
uniform sampler2D dtex;

out vec4 FragColor;

// TODO: add support in the code for custom focal distances
// and foreground blurring (for cutscenes)
uniform float focalDepth;
uniform float range;
uniform bool blurForeground;
float maxblur = 1.;

float validTap(float zCenter, float zSample)
{
    // A tap trying to sample from a pixel with a greatly
    // different depth produces undesirable artifacts.

    // It is usually fine, however, if both source and target
    // are sufficiently distant, but we still need a smooth transition
    // One exception : both the source and target points
    // are sufficently distant to be in the near max blur zone

    // This is the relevant blending factor for most mid-distance pixels
    float depthRatio = max(zCenter, zSample) / max(0.0001, min(zCenter, zSample));
    if (depthRatio < 1.15)
        return 1.0; // Fast-exit - important for performance!

    float similarDepthBlend = 1.0 - smoothstep(1.15, 1.35, depthRatio);

    // This path is mostly for very distant background objects (hills, etc.)
    // where we might actually want blending despite different depths

    float hazeEdge = range + focalDepth;
    float centerHazeFactor = smoothstep (0.75 * hazeEdge, 1.25 * hazeEdge, zCenter);
    float sampleHazeFactor = smoothstep (0.75 * hazeEdge, 1.25 * hazeEdge, zSample);
    float hazeBlend = centerHazeFactor * sampleHazeFactor;

    return max(hazeBlend, similarDepthBlend);
}

float computeDepth(float curdepth, vec2 uv)
{
    vec4 tapPos = u_inverse_projection_matrix * (2.0 * vec4(uv, curdepth, 1.0) - 1.0);
    tapPos /= tapPos.w;
    return tapPos.z;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    float depth = computeDepth(texture(dtex, uv).x, uv);

    vec4 col = texture(tex, uv);
    vec4 colOriginal = col;

    // If foreground blurring is disabled, close pixels
    // are not affected at all, so we can skip the computations
    if (!blurForeground && depth < focalDepth)
    {
        // Switch which line is commented off to visualize pixels using the fast path
        // FragColor = vec4(mix(colOriginal.rgb, vec3(1.0, 0.2, 0.2), 0.5), 1.0);
        FragColor = colOriginal;
        return;
    }

    float blur;
    if (depth > focalDepth)
    {
        blur = min((depth - focalDepth) / range, maxblur);
    }
    else // For foreground blur, we blur more aggressively
    {
        blur = min((focalDepth - depth) / focalDepth, maxblur);
    }

    // The blur radius is scaled with vertical pixels so that it is
    // invariant with regard to track geometry across resolutions
    float scalingFactor = u_screen.y / 1440.0;

    // If you increase the strength significantly, you may want to remove the
    // depth reuse in the loop below, otherwise there will be ghosting artifacts.
    vec2 offset = 12. * scalingFactor / u_screen;

    // Weights derived from http://artmartinsh.blogspot.fr/2010/02/glsl-lens-blur-filter-with-bokeh.html
    // The original weights boil down to sampling on 16 main directions at different distances,
    // 8 weights are used in the 3 closer rings and all 16 on the farthest ring.
    // This sampling had its own unique visual characteristics, but it is also
    // fairly wasteful. We consolidate the offset angle samples on the 0.9 ring. 

    const vec2 main_angles[8] = vec2[8](
        vec2( 0.29, 0.29), vec2( 0.4 , 0.0), vec2( 0.29,-0.29), vec2( 0.0,-0.4),
        vec2(-0.29, 0.29), vec2(-0.4 , 0.0), vec2(-0.29,-0.29), vec2( 0.0, 0.4)
    );

    const vec2 offset_angles[8] = vec2[8](
        vec2( 0.15, 0.37), vec2(-0.37, 0.15), vec2( 0.37,-0.15), vec2(-0.15,-0.37),
        vec2(-0.15, 0.37), vec2( 0.37, 0.15), vec2(-0.37,-0.15), vec2( 0.15,-0.37)
    );

    float successfulTapCount = 1.0;

    // The loop will be unrolled during shader compilation
    for (int i = 0; i < 8; i++)
    {
        // Main angles - we reuse the depth and weight checks
        // This reuse is possible because distances are normally
        // small enough that thin objects present at 0.4/0.7
        // but not at 1.0 are very rare.
        // 1a - Outer ring
        vec2 tapCoordinates = uv + main_angles[i] * offset * blur;
        float tapDepth = computeDepth(texture(dtex, tapCoordinates).x, tapCoordinates);
        float tapWeight = validTap(depth, tapDepth);
        col += tapWeight * texture(tex, tapCoordinates);

        // 1b - 0.7 distance ring
        tapCoordinates = uv + main_angles[i] * (offset * 0.7) * blur;
        col += tapWeight * texture(tex, tapCoordinates);

        // 1c - 0.4 distance ring
        tapCoordinates = uv + main_angles[i] * (offset * 0.4) * blur;
        col += tapWeight * texture(tex, tapCoordinates);

        successfulTapCount += 3.0 * tapWeight;

        // Offset angles - we have to compute the depth and weight again
        // 2 - 0.9 distance ring - double weight for bokeh effect
        tapCoordinates = uv + offset_angles[i] * (offset * 0.9) * blur;
        tapDepth = computeDepth(texture(dtex, tapCoordinates).x, tapCoordinates);
        tapWeight = 2.0 * validTap(depth, tapDepth);
        col += tapWeight * texture(tex, tapCoordinates);
        successfulTapCount += tapWeight;
    }

    col = vec4(col.rgb / successfulTapCount, col.a);
    FragColor = vec4(col.rgb, colOriginal.a);
}
