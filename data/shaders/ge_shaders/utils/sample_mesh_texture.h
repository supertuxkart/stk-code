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
layout(binding = 0) uniform sampler2D f_mesh_texture_0;
layout(binding = 1) uniform sampler2D f_mesh_texture_1;
layout(binding = 2) uniform sampler2D f_mesh_texture_2;
layout(binding = 3) uniform sampler2D f_mesh_texture_3;
layout(binding = 4) uniform sampler2D f_mesh_texture_4;
layout(binding = 5) uniform sampler2D f_mesh_texture_5;
layout(binding = 6) uniform sampler2D f_mesh_texture_6;
layout(binding = 7) uniform sampler2D f_mesh_texture_7;

vec4 sampleMeshTexture0(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_0, uv);
}

vec4 sampleMeshTexture1(int material_id, vec2 uv)
{
    return texture(f_mesh_texture_1, uv);
}

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
