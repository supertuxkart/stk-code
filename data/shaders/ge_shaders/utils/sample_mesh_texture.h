#ifdef BIND_MESH_TEXTURES_AT_ONCE
layout(binding = 0) uniform sampler2D f_mesh_textures[SAMPLER_SIZE * TOTAL_MESH_TEXTURE_LAYER];

vec4 sampleMeshTexture0(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[(TOTAL_MESH_TEXTURE_LAYER * material_id) + 0], uv);
}

vec4 sampleMeshTexture1(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[(TOTAL_MESH_TEXTURE_LAYER * material_id) + 1], uv);
}

vec4 sampleMeshTexture2(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[(TOTAL_MESH_TEXTURE_LAYER * material_id) + 2], uv);
}

vec4 sampleMeshTexture3(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[(TOTAL_MESH_TEXTURE_LAYER * material_id) + 3], uv);
}

vec4 sampleMeshTexture4(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[(TOTAL_MESH_TEXTURE_LAYER * material_id) + 4], uv);
}

vec4 sampleMeshTexture5(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[(TOTAL_MESH_TEXTURE_LAYER * material_id) + 5], uv);
}

vec4 sampleMeshTexture6(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[(TOTAL_MESH_TEXTURE_LAYER * material_id) + 6], uv);
}

vec4 sampleMeshTexture7(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[(TOTAL_MESH_TEXTURE_LAYER * material_id) + 7], uv);
}
#else
layout(binding = 0) uniform sampler2D f_mesh_textures[TOTAL_MESH_TEXTURE_LAYER];

vec4 sampleMeshTexture0(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[0], uv);
}

vec4 sampleMeshTexture1(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[1], uv);
}

#ifdef PBR_ENABLED
vec4 sampleMeshTexture2(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[2], uv);
}

vec4 sampleMeshTexture3(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[3], uv);
}

vec4 sampleMeshTexture4(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[4], uv);
}

vec4 sampleMeshTexture5(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[5], uv);
}

vec4 sampleMeshTexture6(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[6], uv);
}

vec4 sampleMeshTexture7(int material_id, vec2 uv)
{
    return texture(f_mesh_textures[7], uv);
}
#endif
#endif
