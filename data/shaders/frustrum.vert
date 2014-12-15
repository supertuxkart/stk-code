uniform int idx;

layout(location = 0) in vec3 Position;

void main(void)
{
    gl_Position = ShadowViewProjMatrixes[idx] * vec4(Position, 1.);
}
