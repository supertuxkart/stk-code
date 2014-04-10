layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
};

uniform mat4 ModelMatrix;

#if __VERSION__ >= 130
in vec3 Position;
in vec4 Color;
out vec4 color;
#else
attribute vec3 Position;
attribute vec4 Color;
varying vec4 color;
#endif


void main(void)
{
    color = Color.zyxw;
    gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.);
}
