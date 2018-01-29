uniform int idx;

layout(location = 0) in vec3 Position;

void main(void)
{
    gl_Position = u_shadow_projection_view_matrices[idx] * vec4(Position, 1.);
}
