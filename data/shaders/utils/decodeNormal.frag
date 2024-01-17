vec3 DecodeNormal(vec2 n)
{
    n = n * 2.0 - 1.0;
    vec3 ret = vec3(n.x, n.y, 1.0 - abs(n.x) - abs(n.y));
    float t = max(-ret.z, 0.0);
    ret.x += ret.x >= 0.0 ? -t : t;
    ret.y += ret.y >= 0.0 ? -t : t;
    return normalize(ret);
}
