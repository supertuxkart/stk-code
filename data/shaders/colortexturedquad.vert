uniform vec2 center;
uniform vec2 size;
uniform vec2 texcenter;
uniform vec2 texsize;

#if __VERSION__ >= 130
in vec2 position;
in vec2 texcoord;
in uvec4 color;
out vec2 uv;
out vec4 col;
#else
attribute vec2 position;
attribute vec2 texcoord;
attribute uvec4 color;
varying vec2 uv;
varying vec4 col;
#endif


void main()
{
	col = vec4(color) / 255.;
	uv = texcoord * texsize + texcenter;
	gl_Position = vec4(position * size + center, 0., 1.);
}
