uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;
uniform mat4 TextureMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);

#if __VERSION__ >= 130
in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
out vec3 nor;
out vec2 uv;
#else
attribute vec3 Position;
attribute vec3 Normal;
attribute vec2 Texcoord;
varying vec3 nor;
varying vec2 uv;
#endif



void main(void)
{
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
}
