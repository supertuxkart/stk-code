uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;
uniform mat4 TextureMatrix;

#if __VERSION__ >= 130
in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
in vec4 Color;
out vec2 uv;
out vec3 normal;
#else
attribute vec3 Position;
attribute vec3 Normal;
attribute vec2 Texcoord;
attribute vec4 Color;
varying vec2 uv;
varying vec3 normal;
#endif

void main() {
	normal = (TransposeInverseModelView * vec4(Normal, 0)).xyz;
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
