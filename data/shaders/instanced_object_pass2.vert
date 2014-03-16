uniform mat4 ViewProjectionMatrix;
uniform mat4 TextureMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);


in vec3 Origin;

in vec3 Position;
in vec2 Texcoord;
out vec2 uv;



void main(void)
{
    mat4 ModelMatrix = mat4(1.);
    ModelMatrix[3].xyz += Origin;
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
    gl_Position = ViewProjectionMatrix * ModelMatrix * vec4(Position, 1.);
}
