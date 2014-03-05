uniform vec3 windDir;
uniform mat4 ModelViewProjectionMatrix;

#if __VERSION__ >= 130
in vec3 Position;
in vec2 Texcoord;
in vec4 Color;
out vec2 uv;
#else
attribute vec3 Position;
attribute vec2 Texcoord;
attribute vec4 Color;
varying vec2 uv;
#endif

void main()
{
	uv = Texcoord;
	gl_Position = ModelViewProjectionMatrix * vec4(Position + windDir * Color.r, 1.);
}
