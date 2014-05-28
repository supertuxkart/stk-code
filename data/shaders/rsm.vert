uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;
uniform mat4 RSMMatrix;

uniform mat4 TextureMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);


in vec3 Position;
in vec2 Texcoord;
in vec3 Normal;
out vec3 nor;
out vec2 uv;


void main(void)
{
    mat4 ModelViewProjectionMatrix = RSMMatrix * ModelMatrix;
    mat4 TransposeInverseModel = transpose(InverseModelMatrix);
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
    nor = (vec4(Normal, 0.)).xyz;
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
}
