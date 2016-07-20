vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix)
{
    vec4 pos = 2.0 * vec4(uvDepth, 1.0) - 1.0;
    pos.xy *= vec2(InverseProjectionMatrix[0][0], InverseProjectionMatrix[1][1]);
    pos.zw = vec2(pos.z * InverseProjectionMatrix[2][2] + pos.w, pos.z * InverseProjectionMatrix[2][3] + pos.w);
    pos /= pos.w;
    return pos;
}
