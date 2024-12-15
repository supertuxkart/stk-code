layout(std140, set = 1, binding = 0) uniform CameraBuffer
{
    mat4 m_view_matrix;
    mat4 m_projection_matrix;
    mat4 m_inverse_view_matrix;
    mat4 m_inverse_projection_matrix;
    mat4 m_projection_view_matrix;
    mat4 m_inverse_projection_view_matrix;
    vec4 m_viewport;
    vec3  m_ambient_color;
    float m_sun_angle_tan_half;
    vec3  m_sun_color;
    float m_sun_scatter;
    vec3  m_sun_direction; // Premultipled by transposed inverse view matrix
    float m_fog_density;
    vec4  m_fog_color;
    mat4 m_shadow_matrix[3];
    vec4 m_warp_strength;
} u_camera;