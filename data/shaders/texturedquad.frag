#version 130
uniform sampler2D tex;

in vec2 uv;

void main()
{
	gl_FragColor = texture(tex, uv);
}