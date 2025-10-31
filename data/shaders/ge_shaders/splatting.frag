layout(location = 1) in vec2 f_uv;
layout(location = 2) in vec2 f_uv_two;
layout(location = 3) flat in int f_material_id;
layout(location = 5) in vec3 f_normal;
layout(location = 8) in vec4 f_world_position;

layout(location = 0) out vec4 o_color;
layout(location = 1) out vec4 o_normal;

#include "utils/sample_mesh_texture.glsl"
#include "utils/handle_pbr.glsl"
#include "../utils/encode_normal.frag"

#define HIGH_RES_SAMPLER 1.0f
#define LOW_RES_SAMPLER 0.5f

#ifdef PBR_ENABLED
vec4 sampleMultiResTextureLayer2(float factor, int material_id, vec2 uv)
{
    return mix(sampleMeshTexture2(material_id, uv * HIGH_RES_SAMPLER), sampleMeshTexture2(material_id, uv * LOW_RES_SAMPLER), factor);
}

vec4 sampleMultiResTextureLayer3(float factor, int material_id, vec2 uv)
{
    return mix(sampleMeshTexture3(material_id, uv * HIGH_RES_SAMPLER), sampleMeshTexture3(material_id, uv * LOW_RES_SAMPLER), factor);
}

vec4 sampleMultiResTextureLayer4(float factor, int material_id, vec2 uv)
{
    return mix(sampleMeshTexture4(material_id, uv * HIGH_RES_SAMPLER), sampleMeshTexture4(material_id, uv * LOW_RES_SAMPLER), factor);
}

vec4 sampleMultiResTextureLayer5(float factor, int material_id, vec2 uv)
{
    return mix(sampleMeshTexture5(material_id, uv * HIGH_RES_SAMPLER), sampleMeshTexture5(material_id, uv * LOW_RES_SAMPLER), factor);
}
#endif

void main()
{
#ifdef PBR_ENABLED
    // mitigate repetitive patterns
    float cam_dist = length(u_camera.m_view_matrix * f_world_position);
    float mitigation = clamp(pow(cam_dist * 0.01, 2.0) - 0., 0., 1.);

    // Splatting part
    vec4 splatting = sampleMeshTexture1(f_material_id, f_uv_two);
    vec4 detail0 = sampleMultiResTextureLayer2(mitigation, f_material_id, f_uv);
    vec4 detail1 = sampleMultiResTextureLayer3(mitigation, f_material_id, f_uv);
    vec4 detail2 = sampleMultiResTextureLayer4(mitigation, f_material_id, f_uv);
    vec4 detail3 = sampleMultiResTextureLayer5(mitigation, f_material_id, f_uv);

    vec4 splatted = splatting.r * detail0 +
        splatting.g * detail1 +
        splatting.b * detail2 +
        max(0.0, (1.0 - splatting.r - splatting.g - splatting.b)) * detail3;

    vec3 normal = normalize(f_normal.xyz);
    if (u_deferred)
    {
        o_color = vec4(splatted.xyz, 0.0);
        o_normal.xy = EncodeNormal(normal);
        o_normal.zw = vec2(0.0);
    }
    else
    {
        o_color = vec4(handlePBR(splatted.xyz, vec3(0.0), f_world_position,
            normal), 1.0);
    }
#endif
}
