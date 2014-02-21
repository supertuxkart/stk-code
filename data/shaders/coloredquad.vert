#version 330
uniform vec2 center;
uniform vec2 size;

in vec2 position;

void main()
{
	gl_Position = vec4(position * size + center, 0., 1.);
}