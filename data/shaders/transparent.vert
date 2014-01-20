#version 130
uniform mat4 ModelViewProjectionMatrix;

in vec3 Position;
in vec2 Texcoord;
out vec2 uv;

void main()
{
	uv = Texcoord;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
