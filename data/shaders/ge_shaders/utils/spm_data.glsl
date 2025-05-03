#ifdef BIND_MESH_TEXTURES_AT_ONCE
#extension GL_ARB_shader_draw_parameters : enable
#endif
struct ObjectData
{
    vec3 m_translation;
    float m_hue_change;
    vec4 m_rotation;
    vec3 m_scale;
    uint m_custom_vertex_color;
    int m_skinning_offset;
    int m_material_id;
    vec2 m_texture_trans;
};

layout(std140, set = 1, binding = 1) readonly buffer ObjectBuffer
{
    ObjectData m_objects[];
} u_object_buffer;

layout(std140, set = 1, binding = 2) readonly buffer SkinningMatrices
{
    mat4 m_mat[];
} u_skinning_matrices;

#ifdef BIND_MESH_TEXTURES_AT_ONCE
layout(std430, set = 1, binding = 4) readonly buffer MaterialIDs
{
    int m_material_id[];
} u_material_ids;
#endif
