#version 330
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;
uniform mat4 TextureMatrix;

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
in vec4 Color;
out vec2 uv;
noperspective out vec3 normal;

void main() {
	normal = (TransposeInverseModelView * vec4(Normal, 0)).xyz;
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
