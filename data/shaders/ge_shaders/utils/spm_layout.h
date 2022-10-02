#ifdef BIND_MESH_TEXTURES_AT_ONCE
#extension GL_ARB_shader_draw_parameters : enable
#endif

layout(std140, set = 1, binding = 0) uniform CameraBuffer
{
    mat4 m_view_matrix;
    mat4 m_projection_matrix;
    mat4 m_inverse_view_matrix;
    mat4 m_inverse_projection_matrix;
    mat4 m_projection_view_matrix;
    mat4 m_inverse_projection_view_matrix;
} u_camera;

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
layout(std430, set = 1, binding = 3) readonly buffer MaterialIDs
{
    int m_material_id[];
} u_material_ids;
#endif

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec4 v_normal;
layout(location = 2) in vec4 v_color;
layout(location = 3) in vec2 v_uv;
layout(location = 4) in vec2 v_uv_two;
layout(location = 5) in vec4 v_tangent;
layout(location = 6) in ivec4 v_joint;
layout(location = 7) in vec4 v_weight;

layout(location = 0) out vec4 f_vertex_color;
layout(location = 1) out vec2 f_uv;
layout(location = 2) out vec2 f_uv_two;
layout(location = 3) flat out int f_material_id;
layout(location = 4) out float f_hue_change;
