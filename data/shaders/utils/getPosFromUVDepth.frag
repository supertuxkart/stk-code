vec4 getPosFromUVDepth(vec3 uvDepth, mat4 u_inverse_projection_matrix)
{
    vec4 pos = 2.0 * vec4(uvDepth, 1.0) - 1.0;
    pos.xy *= vec2(u_inverse_projection_matrix[0][0], u_inverse_projection_matrix[1][1]);
    pos.zw = vec2(pos.z * u_inverse_projection_matrix[2][2] + pos.w, pos.z * u_inverse_projection_matrix[2][3] + pos.w);
    pos /= pos.w;
    return pos;
}
