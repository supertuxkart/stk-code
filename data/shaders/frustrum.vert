uniform int idx;

in vec3 Position;

void main(void)
{
    gl_Position = ShadowViewProjMatrixes[idx] * vec4(Position, 1.);
}
