#version 130
uniform sampler2D texture;

in vec2 uv;
in vec4 col;

void main()
{
	gl_FragColor = texture2D(texture, uv) * col;
}
