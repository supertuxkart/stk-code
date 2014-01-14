#version 130
uniform ivec4 color;

void main()
{
	gl_FragColor = vec4(color) / 255.;
}
