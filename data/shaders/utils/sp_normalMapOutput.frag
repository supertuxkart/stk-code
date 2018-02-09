// This function encapsulate the computation of normal maps
in vec3 tangent;
in vec3 bitangent;
in vec3 normal;

void outputNormalMapPbrData(vec3 layer_3, vec3 layer_2)
{
    vec3 tangent_space_normal = 2.0 * layer_3.xyz - 1.0;
    vec3 frag_tangent = normalize(tangent);
    vec3 frag_bitangent = normalize(bitangent);
    vec3 frag_normal = normalize(normal);
    mat3 t_b_n = mat3(frag_tangent, frag_bitangent, frag_normal);

    vec3 world_normal = t_b_n * tangent_space_normal;

    o_normal_depth.xy = 0.5 * EncodeNormal(normalize(world_normal)) + 0.5;
    o_normal_depth.z = layer_2.x;
    o_gloss_map = layer_2.yz;
}


