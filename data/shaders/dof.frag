uniform sampler2D tex;
uniform sampler2D dtex;

out vec4 FragColor;

// TODO: add support in the code for custom focal distances
// and foreground blurring (for cutscenes)
uniform float focalDepth = 72.;
float maxblur = 1.;
uniform float range = 360.;
uniform bool blurForeground = false;
float baselineHeight = 1440; // Used to tune blur radius with resolution

void main()
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    float curdepth = texture(dtex, uv).x;
    vec4 FragPos = u_inverse_projection_matrix * (2.0 * vec4(uv, curdepth, 1.0) - 1.0);
    FragPos /= FragPos.w;

    vec4 col = texture(tex, uv);
    vec4 colOriginal = col;
    float depth = FragPos.z;

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

    float scalingFactor = u_screen.y / 1440;
    vec2 offset = 12. * scalingFactor / u_screen;

    // Weight from here http://artmartinsh.blogspot.fr/2010/02/glsl-lens-blur-filter-with-bokeh.html

    const vec2 main_angles[8] = vec2[8](
        vec2( 0.29, 0.29), vec2( 0.4 , 0.0), vec2( 0.29,-0.29), vec2( 0.0,-0.4),
        vec2(-0.29, 0.29), vec2(-0.4 , 0.0), vec2(-0.29,-0.29), vec2( 0.0, 0.4)
    );

    const vec2 offset_angles[8] = vec2[8](
        vec2( 0.15, 0.37), vec2(-0.37, 0.15), vec2( 0.37,-0.15), vec2(-0.15,-0.37),
        vec2(-0.15, 0.37), vec2( 0.37, 0.15), vec2(-0.37,-0.15), vec2( 0.15,-0.37)
    );

    // The loop will be unrolled during shader compilation
    for (int i = 0; i < 8; i++)
    {
        // 1) Sample Depth
        col += texture(tex, uv + main_angles[i] * offset * blur);
        col += texture(tex, uv + offset_angles[i] * offset * blur);
        col += texture(tex, uv + offset_angles[i] * (offset * 0.9) * blur);
        col += texture(tex, uv + main_angles[i] * (offset * 0.7) * blur);
        col += texture(tex, uv + main_angles[i] * (offset * 0.4) * blur);
    }

    col = vec4(col.rgb / 41.0, col.a);
    FragColor = vec4(col.rgb, colOriginal.a);
}
