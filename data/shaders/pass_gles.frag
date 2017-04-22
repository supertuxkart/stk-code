#version 300 es

precision mediump float;

uniform sampler2D tex;

in vec2 uv;
out vec4 FragColor;

void main()
{
	FragColor = texture(tex, uv);
}
