vec2 getDisplaceShift(float horiz, float vert)
{
    vec2 offset = vec2(horiz, vert);
    offset = 2.0 * offset - 1.0;

    vec4 shiftval;
    shiftval.r = step(offset.x, 0.0) * -offset.x;
    shiftval.g = step(0.0, offset.x) * offset.x;
    shiftval.b = step(offset.y, 0.0) * -offset.y;
    shiftval.a = step(0.0, offset.y) * offset.y;

    vec2 shift;
    shift.x = -shiftval.x + shiftval.y;
    shift.y = -shiftval.z + shiftval.w;
    return shift;
}

ivec2 getDisplaceUV(vec2 shift, vec4 viewport, sampler2D displace_mask)
{
    ivec2 uv = ivec2(gl_FragCoord.xy);
    shift *= 0.02 * viewport.zw;
    ivec2 lo = ivec2(viewport.xy);
    ivec2 hi = ivec2(viewport.xy + viewport.zw);
    ivec2 suv = clamp(ivec2(gl_FragCoord.xy) + ivec2(shift), lo, hi);
    vec2 new_mask = texelFetch(displace_mask, suv, 0).xy;
    if (!(new_mask.x == 0.0 && new_mask.y == 0.0))
        uv = suv;
    return uv;
}
