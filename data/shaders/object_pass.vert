uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;
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
out vec3 nor;
out vec2 uv;
out vec2 uv_bis;
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
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
    uv_bis = SecondTexcoord;
}
