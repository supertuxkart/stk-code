uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;
uniform mat4 RSMMatrix;

uniform mat4 TextureMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);


layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 3) in vec2 Texcoord;

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
