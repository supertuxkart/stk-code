#version 330
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;
uniform mat4 TextureMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
noperspective out vec3 nor;
out vec2 uv;

void main(void)
{
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (TransposeInverseModelView * vec4(Normal, 0.)).xyz;
}
