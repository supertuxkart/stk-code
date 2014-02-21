#version 330
uniform sampler2D tex;
uniform sampler2D dtex;

uniform int viz;

in vec2 uv;
out vec4 FragColor;

void main()
{
	vec2 tc = uv;

	vec4 shiftval = texture(dtex, tc) / vec4(50.0);
	vec2 shift;
	shift.x = -shiftval.x + shiftval.y;
	shift.y = -shiftval.z + shiftval.w;

	tc += shift;

	vec4 newcol = texture(tex, tc);

	if (viz < 1)
	{
		FragColor = newcol;
	} else
	{
		FragColor = shiftval * vec4(50.0);
	}
}
