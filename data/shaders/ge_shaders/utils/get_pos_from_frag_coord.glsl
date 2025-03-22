vec3 getPosFromFragCoord(vec4 frag_coord, vec4 viewport, mat4 inverse_projection_matrix)
{
    return vec3(
        ((frag_coord.x - viewport.x) / viewport.z * 2.0 - 1.0) * inverse_projection_matrix[0][0],
        ((frag_coord.y - viewport.y) / viewport.w * 2.0 - 1.0) * inverse_projection_matrix[1][1],
        inverse_projection_matrix[3][2]
    ) / frag_coord.w;
}
