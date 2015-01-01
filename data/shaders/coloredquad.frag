uniform ivec4 color;

out vec4 FragColor;

void main()
{
	FragColor = vec4(color) / 255.;
}
