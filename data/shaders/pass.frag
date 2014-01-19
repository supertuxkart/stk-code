#version 130
uniform sampler2D tex;

out vec4 FragColor;

void main()
{
	FragColor = texture(tex, gl_TexCoord[0].xy);
}
