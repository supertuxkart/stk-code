#version 330 compatibility
uniform float power;
uniform sampler2D tex;

out vec4 FragColor;

void main()
{
	vec4 col = texture(tex, gl_TexCoord[0].xy);
	if (col.a < 0.5)
		discard;

	FragColor = vec4(col.xyz, power);
}
