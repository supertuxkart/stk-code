#version 130
uniform vec2 center;
uniform vec2 size;
uniform vec2 texcenter;
uniform vec2 texsize;

in vec2 position;
in vec2 texcoord;
in ivec4 color;
out vec2 uv;
out vec4 col;

void main()
{
	col = vec4(color) / 255.;
	uv = texcoord * texsize + texcenter;
	gl_Position = vec4(position * size + center, 0., 1.);
}
