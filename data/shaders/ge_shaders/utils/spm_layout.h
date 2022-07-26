layout(std140, set = 1, binding = 0) uniform CameraBuffer
{
    mat4 m_view_matrix;
    mat4 m_projection_matrix;
    mat4 m_inverse_view_matrix;
    mat4 m_inverse_projection_matrix;
    mat4 m_projection_view_matrix;
} u_camera;

struct ObjectData
{
    mat4 m_model;
    int m_skinning_offest;
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
layout(location = 2) flat out int f_material_id;
