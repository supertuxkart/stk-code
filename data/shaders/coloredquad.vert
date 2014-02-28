uniform vec2 center;
uniform vec2 size;

#if __VERSION__ >= 130
in vec2 position;
#else
attribute vec2 position;
#endif


void main()
{
	gl_Position = vec4(position * size + center, 0., 1.);
}