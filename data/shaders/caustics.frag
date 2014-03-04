uniform sampler2D tex;
uniform sampler2D caustictex;
uniform vec2 dir;
uniform vec2 dir2;

in vec2 uv;
out vec4 FragColor;

void main()
{
	vec2 tc = uv;

	vec3 col = texture(tex, tc).xyz;
	float caustic = texture(caustictex, tc + dir).x;
	float caustic2 = texture(caustictex, (tc.yx + dir2 * vec2(-0.6, 0.3)) * vec2(0.6)).x;

	col += caustic * caustic2;

	FragColor = vec4(col, 1.0);
}
