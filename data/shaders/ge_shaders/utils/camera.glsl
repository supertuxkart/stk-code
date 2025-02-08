layout(std140, set = 1, binding = 0) uniform CameraBuffer
{
    mat4 m_view_matrix;
    mat4 m_projection_matrix;
    mat4 m_inverse_view_matrix;
    mat4 m_inverse_projection_matrix;
    mat4 m_projection_view_matrix;
    mat4 m_inverse_projection_view_matrix;
    vec4 m_viewport;
    mat4 m_shadow_matrix[3];
    mat4 m_shadow_view_matrix;
    vec3 _margin;
    float m_light_count;
} u_camera;