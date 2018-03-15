vec3 rotateVector(vec4 quat, vec3 vec)
{
    return vec + 2.0 * cross(cross(vec, quat.xyz) + quat.w * vec, quat.xyz);
}

vec4 getWorldPosition(vec3 origin, vec4 rotation, vec3 scale, vec3 local_pos)
{
    local_pos = local_pos * scale;
    local_pos = rotateVector(rotation, local_pos);
    local_pos = local_pos + origin;
    return vec4(local_pos, 1.0);
}

vec4 convert10BitVector(vec4 orig)
{
    vec4 ret;
    ret.x = orig.x * 0.00195694715;
    ret.y = orig.y * 0.00195694715;
    ret.z = orig.z * 0.00195694715;
    ret.w = max(orig.w, -1.0);
    return ret;
}
