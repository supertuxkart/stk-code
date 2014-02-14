#version 330
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 ModelViewMatrix;

in vec3 Position;
in vec2 Texcoord;
in vec2 SecondTexcoord;
out vec2 uv;
out vec2 uv_bis;
out float camdist;

void main() {
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
	uv = Texcoord;
	uv_bis = SecondTexcoord;
	camdist = length(ModelViewMatrix *  vec4(Position, 1.));
}
