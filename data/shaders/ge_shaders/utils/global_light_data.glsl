layout(std140, set = 3, binding = 0) uniform GlobalLightBuffer
{
    vec3  m_ambient_color;
    float m_sun_scatter;
    vec3  m_sun_color;
    float m_sun_angle_tan_half;
    vec3  m_sun_direction;
    float m_fog_density;
    vec4  m_fog_color;
} u_global_light;

layout(set = 2, binding = 0) uniform samplerCube u_diffuse_environment_map;
layout(set = 2, binding = 1) uniform samplerCube u_specular_environment_map;
layout(set = 3, binding = 1) uniform sampler2DArrayShadow u_shadow_map;