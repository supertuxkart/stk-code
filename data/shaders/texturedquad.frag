#version 130
uniform sampler2D texture;

in vec2 uv;

void main()
{
	gl_FragColor = texture2D(texture, uv);
}