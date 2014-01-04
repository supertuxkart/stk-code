#version 130
uniform vec2 center;
uniform vec2 size;

in vec2 position;
in vec2 texcoord;
out vec2 tc;

void main()
{
	tc = texcoord;
	gl_Position = vec4(position * size + center, 0., 1.);
}