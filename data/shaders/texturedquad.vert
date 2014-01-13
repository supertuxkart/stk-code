#version 130
uniform vec2 center;
uniform vec2 size;
uniform vec2 texcenter;
uniform vec2 texsize;

in vec2 position;
in vec2 texcoord;
out vec2 uv;

void main()
{
	uv = texcoord * texsize + texcenter;
	gl_Position = vec4(position * size + center, 0., 1.);
}