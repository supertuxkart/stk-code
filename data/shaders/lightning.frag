uniform vec3 intensity;
out vec4 FragColor;

void main()
{
	FragColor = vec4(intensity, 1.0f);
}
