#version 130
uniform mat4 ModelViewProjectionMatrix;

in vec3 Position;
in vec2 Texcoord;
in vec2 SecondTexcoord;
out vec2 uv;
out vec2 uv_bis;

void main(void)
{
    uv = Texcoord;
	uv_bis = SecondTexcoord;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
