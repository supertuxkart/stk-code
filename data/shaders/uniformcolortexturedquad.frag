uniform sampler2D tex;
uniform ivec4 color;

in vec2 uv;
out vec4 FragColor;

void main()
{
	vec4 res = texture(tex, uv);
	FragColor = res * vec4(color) / 255.;
}
