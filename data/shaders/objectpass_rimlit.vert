#version 130
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
in vec4 Color;
out vec2 uv;
noperspective out vec3 normal;

void main() {
	normal = (TransposeInverseModelView * vec4(Normal, 0)).xyz;
	uv = Texcoord;
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
