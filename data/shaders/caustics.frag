#version 330 compatibility
uniform sampler2D tex;
uniform sampler2D caustictex;
uniform vec2 dir;
uniform vec2 dir2;

out vec4 FragColor;

void main()
{
	vec2 tc = gl_TexCoord[0].xy;

	vec3 col = texture(tex, tc).xyz;
	float caustic = texture(caustictex, tc + dir).x;
	float caustic2 = texture(caustictex, (tc.yx + dir2 * vec2(-0.6, 0.3)) * vec2(0.6)).x;

	col += caustic * caustic2 * 10.0;

	FragColor = vec4(col, 1.0);
}
