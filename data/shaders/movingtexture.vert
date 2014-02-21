#version 330
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TextureMatrix;

in vec3 Position;
in vec2 Texcoord;
in vec2 SecondTexcoord;
out vec2 uv;
out vec2 uv_bis;

void main(void)
{
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
	uv_bis = SecondTexcoord;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
