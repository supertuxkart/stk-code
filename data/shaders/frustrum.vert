layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

uniform int idx;

in vec3 Position;

void main(void)
{
    gl_Position = ShadowViewProjMatrixes[idx] * vec4(Position, 1.);
}
