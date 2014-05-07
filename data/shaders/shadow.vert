layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
};

uniform mat4 ModelMatrix;

in vec3 Position;
in vec2 Texcoord;

out vec2 tc;
out int layerId;

void main(void)
{
    layerId = gl_InstanceID & 3;
    tc = Texcoord;
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * vec4(Position, 1.);
}
