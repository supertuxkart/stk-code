layout(push_constant) uniform Constants
{
    mat4 m_inverse_projection_view_matrix;
} u_push_constants;

layout(location = 0) in vec2 f_uv;
layout(binding = 0) uniform samplerCube f_skybox_texture;

layout(location = 0) out vec4 o_color;

void main()
{
    vec2 uv = 2.0f * f_uv - 1.0f;
    vec4 front = u_push_constants.m_inverse_projection_view_matrix * vec4(uv, -1.0, 1.0);
    vec4 back = u_push_constants.m_inverse_projection_view_matrix * vec4(uv, 1.0, 1.0);
    vec3 dir = back.xyz / back.w - front.xyz / front.w;
    o_color = texture(f_skybox_texture, dir);
}
