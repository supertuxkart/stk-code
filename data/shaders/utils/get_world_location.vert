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

vec4 convert10BitVector(int pked)
{
    vec4 ret;
    int part = pked & 1023;
    float part_mix = float(clamp(int(part & 512), 0, 1));
    ret.x = mix(float(part), float(-1024 + part), part_mix) * 0.00195694715;

    part = (pked >> 10) & 1023;
    part_mix = float(clamp(int(part & 512), 0, 1));
    ret.y = mix(float(part), float(-1024 + part), part_mix) * 0.00195694715;

    part = (pked >> 20) & 1023;
    part_mix = float(clamp(int(part & 512), 0, 1));
    ret.z = mix(float(part), float(-1024 + part), part_mix) * 0.00195694715;

    part = pked >> 30;
    part_mix = float(clamp(int(part & 2), 0, 1));
    ret.w = mix(float(part), -1.0f, part_mix);

    return ret;
}
