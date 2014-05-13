uniform vec2 center;
uniform vec2 size;
uniform vec2 texcenter;
uniform vec2 texsize;

#if __VERSION__ >= 130
in vec2 position;
in vec2 texcoord;
out vec2 uv;
#else
attribute vec2 position;
attribute vec2 texcoord;
varying vec2 uv;
#endif


void main()
{
	uv = texcoord * texsize + texcenter;
	gl_Position = vec4(position * size + center, 0., 1.);
}