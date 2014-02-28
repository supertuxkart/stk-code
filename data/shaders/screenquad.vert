in vec2 Position;
in vec2 Texcoord;

#if __VERSION__ >= 130
out vec2 uv;
#else
varying vec2 uv;
#endif


void main() {
	uv = Texcoord;
	gl_Position = vec4(Position, 0., 1.);
}
