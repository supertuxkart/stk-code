#version 130
uniform sampler2D tex;
uniform sampler2D oldtex;
uniform vec2 pixel;
uniform vec2 multi;
uniform int size;

void main()
{
	float res = 0.0;
	vec2 tc = gl_TexCoord[0].xy;
//	tc.y = 1.0 - tc.y;
	tc *= multi;

	for (int i = 0; i < size; i++)
	{
		float col = texture2D(tex, tc).x;
		res = max(col, res);

		tc += pixel;
	}

	float old = texture2D(oldtex, gl_TexCoord[0].xy).x;

	gl_FragColor = vec4(mix(old, res, 0.7));
}
