#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
#else
layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};
#endif

uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;

uniform mat4 TextureMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);

#if __VERSION__ >= 130
in vec3 Position;
in vec2 Texcoord;
in vec2 SecondTexcoord;
in vec3 Normal;
in vec4 Color;
out vec3 nor;
out vec2 uv;
out vec2 uv_bis;
out vec4 color;
#else
attribute vec3 Position;
attribute vec3 Normal;
attribute vec2 Texcoord;
attribute vec2 SecondTexcoord;
varying vec3 nor;
varying vec2 uv;
varying vec2 uv_bis;
#endif


void main(void)
{
    color = Color.zyxw;
    mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
    mat4 TransposeInverseModelView = transpose(InverseModelMatrix * InverseViewMatrix);
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
    uv_bis = SecondTexcoord;
}
