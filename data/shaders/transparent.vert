#version 330
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TextureMatrix;

in vec3 Position;
in vec2 Texcoord;
out vec2 uv;

void main()
{
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
