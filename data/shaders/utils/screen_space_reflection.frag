vec3 CalcCoordFromPosition(vec3 pos, mat4 projection_matrix,
                           vec2 viewport_scale, vec2 viewport_offset)
{
    vec4 projectedCoord = projection_matrix * vec4(pos, 1.0);
    projectedCoord.xyz /= projectedCoord.w;
#if defined(VULKAN)
    projectedCoord.xy   = projectedCoord.xy * 0.5 + 0.5;  // map X,Y from -1 -> +1 into 0 -> 1
    // no Z remap here because vulkan projection matrix already gave us Z in [0..1]
#else
    projectedCoord.xyz  = projectedCoord.xyz * 0.5 + 0.5;
#endif
    // scale and offset by viewport
    projectedCoord.xy   = projectedCoord.xy * viewport_scale + viewport_offset;
    return projectedCoord.xyz;
}

// Fade out edges of screen buffer tex
// 1 means full render tex, 0 means full IBL tex
float GetEdgeFade(vec2 coords, vec2 viewport_scale, vec2 viewport_offset)
{
    // transform coords to viewport space
    vec2 viewport_coords = (coords - viewport_offset) / viewport_scale;
    float gradL = smoothstep(0.0, 0.4, viewport_coords.x);
    float gradR = 1.0 - smoothstep(0.6, 1.0, viewport_coords.x);
    float gradT = smoothstep(0.0, 0.4, viewport_coords.y);
    float gradB = 1.0 - smoothstep(0.6, 1.0, viewport_coords.y);
    return min(min(gradL, gradR), min(gradT, gradB));
}

vec2 RayCast(vec3 dir, vec3 hitCoord, mat4 projection_matrix,
             vec2 viewport_scale, vec2 viewport_offset, sampler2DShadow depth)
{
    dir *= 0.5;
    hitCoord += dir;

    vec3 projectedCoord = CalcCoordFromPosition(hitCoord, projection_matrix,
                          viewport_scale, viewport_offset);
    float factor = 1.0;

    for (int i = 0; i < 32; i++)
    {
        float direction = texture(depth, projectedCoord);
        factor *= direction;
        dir = dir * (0.5 + 0.5 * factor);
        hitCoord += dir * (2. * direction - 1.);
        projectedCoord = CalcCoordFromPosition(hitCoord, projection_matrix,
                         viewport_scale, viewport_offset);
    }

    return projectedCoord.xy;
}
