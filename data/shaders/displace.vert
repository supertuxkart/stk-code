uniform mat4 ModelViewProjectionMatrix;
uniform mat4 ModelViewMatrix;

#if __VERSION__ >= 130
in vec3 Position;
in vec2 Texcoord;
in vec2 SecondTexcoord;
out vec2 uv;
out vec2 uv_bis;
out float camdist;
#else
attribute vec3 Position;
attribute vec2 Texcoord;
attribute vec2 SecondTexcoord;
varying vec2 uv;
varying vec2 uv_bis;
varying float camdist;
#endif



void main() {
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
	uv = Texcoord;
	uv_bis = SecondTexcoord;
	camdist = length(ModelViewMatrix *  vec4(Position, 1.));
}
