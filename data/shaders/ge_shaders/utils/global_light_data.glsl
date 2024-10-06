layout(std140, set = 2, binding = 0) uniform GlobalLightBuffer
{
    vec3  m_ambient_color;
    float m_sun_angle_tan_half;
    vec3  m_sun_color;
    float m_sun_scatter;
    vec3  m_sun_direction; // Premultipled by transposed inverse view matrix
    float m_fog_density;
    vec4  m_fog_color;
} u_global_light;