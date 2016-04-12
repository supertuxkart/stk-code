// from Crytek "a bit more deferred CryEngine"
vec2 EncodeNormal(vec3 n)
{
    return normalize(n.xy) * sqrt(n.z * 0.5 + 0.5);
}