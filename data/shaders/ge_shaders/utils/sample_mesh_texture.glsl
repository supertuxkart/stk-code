#ifdef BIND_MESH_TEXTURES_AT_ONCE
layout(binding = 0) uniform sampler2D f_mesh_textures[SAMPLER_SIZE * TOTAL_MESH_TEXTURE_LAYER];

vec4 sampleMeshTexture0(int material_id, vec2 uv)
{
    int id = (TOTAL_MESH_TEXTURE_LAYER * material_id) + 0;
    return texture(f_mesh_textures[GE_SAMPLE_TEX_INDEX(id)], uv);
}

vec4 sampleMeshTexture1(int material_id, vec2 uv)
{
    int id = (TOTAL_MESH_TEXTURE_LAYER * material_id) + 1;
    return texture(f_mesh_textures[GE_SAMPLE_TEX_INDEX(id)], uv);
}

vec4 sampleMeshTexture2(int material_id, vec2 uv)
{
    int id = (TOTAL_MESH_TEXTURE_LAYER * material_id) + 2;
    return texture(f_mesh_textures[GE_SAMPLE_TEX_INDEX(id)], uv);
}

vec4 sampleMeshTexture3(int material_id, vec2 uv)
{
    int id = (TOTAL_MESH_TEXTURE_LAYER * material_id) + 3;
    return texture(f_mesh_textures[GE_SAMPLE_TEX_INDEX(id)], uv);
}

vec4 sampleMeshTexture4(int material_id, vec2 uv)
{
    int id = (TOTAL_MESH_TEXTURE_LAYER * material_id) + 4;
    return texture(f_mesh_textures[GE_SAMPLE_TEX_INDEX(id)], uv);
}

vec4 sampleMeshTexture5(int material_id, vec2 uv)
{
    int id = (TOTAL_MESH_TEXTURE_LAYER * material_id) + 5;
    return texture(f_mesh_textures[GE_SAMPLE_TEX_INDEX(id)], uv);
}

vec4 sampleMeshTexture6(int material_id, vec2 uv)
{
    int id = (TOTAL_MESH_TEXTURE_LAYER * material_id) + 6;
    return texture(f_mesh_textures[GE_SAMPLE_TEX_INDEX(id)], uv);
}

vec4 sampleMeshTexture7(int material_id, vec2 uv)
{
    int id = (TOTAL_MESH_TEXTURE_LAYER * material_id) + 7;
    return texture(f_mesh_textures[GE_SAMPLE_TEX_INDEX(id)], uv);
}
#else
layout(binding = 0) uniform sampler2D f_mesh_texture_0;
layout(binding = 1) uniform sampler2D f_mesh_texture_1;
#ifdef PBR_ENABLED
layout(binding = 2) uniform sampler2D f_mesh_texture_2;
layout(binding = 3) uniform sampler2D f_mesh_texture_3;
layout(binding = 4) uniform sampler2D f_mesh_texture_4;
layout(binding = 5) uniform sampler2D f_mesh_texture_5;
layout(binding = 6) uniform sampler2D f_mesh_texture_6;
layout(binding = 7) uniform sampler2D f_mesh_texture_7;
#endif

vec4 sampleMeshTexture0(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_0, uv);
}

vec4 sampleMeshTexture1(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_1, uv);
}

#ifdef PBR_ENABLED
vec4 sampleMeshTexture2(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_2, uv);
}

vec4 sampleMeshTexture3(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_3, uv);
}

vec4 sampleMeshTexture4(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_4, uv);
}

vec4 sampleMeshTexture5(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_5, uv);
}

vec4 sampleMeshTexture6(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_6, uv);
}

vec4 sampleMeshTexture7(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_7, uv);
}
#endif
#endif
