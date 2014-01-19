#version 130
uniform sampler2D tex;

in vec2 uv;
out vec4 FragColor;

void main()
{
	vec4 col = texture(tex, uv);

	col.xyz *= 10.0 * col.a;

	FragColor = vec4(col.xyz, 1.);
}
