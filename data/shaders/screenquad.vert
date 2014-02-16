#version 330

in vec2 Position;
in vec2 Texcoord;
out vec2 uv;

void main() {
	uv = Texcoord;
	gl_Position = vec4(Position, 0., 1.);
}
