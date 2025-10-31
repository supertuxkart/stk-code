vec3 getPosFromFragCoord(vec4 frag_coord, vec4 viewport, mat4 inverse_projection_matrix)
{
    vec2 ndc = vec2((frag_coord.x - viewport.x) / viewport.z * 2.0 - 1.0,
        (frag_coord.y - viewport.y) / viewport.w * 2.0 - 1.0);
    vec4 clip = vec4(ndc, 1.0, 1.0);
    vec4 view_space = inverse_projection_matrix * clip;
    return view_space.xyz / frag_coord.w;
}

vec3 getPosFromUVDepth(vec3 uv_depth, vec4 viewport, mat4 inverse_projection_matrix)
{
    vec2 ndc = vec2((uv_depth.x - viewport.x) / viewport.z * 2.0 - 1.0,
        (uv_depth.y - viewport.y) / viewport.w * 2.0 - 1.0);
    vec4 clip = vec4(ndc, uv_depth.z, 1.0);
    vec4 view_space = inverse_projection_matrix * clip;
    return view_space.xyz / view_space.w;
}
