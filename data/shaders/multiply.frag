#version 130
uniform sampler2D tex1;
uniform sampler2D tex2;

out vec4 FragColor;

void main()
{
	vec4 col1 = texture(tex1, gl_TexCoord[0].xy);
	vec4 col2 = vec4(vec3(texture(tex2, gl_TexCoord[0].xy).x), 1.0);

	FragColor = col1 * col2;
}
