#version 130
uniform sampler2D tex;

in vec2 uv;

void main()
{
	vec4 col = texture2D(tex, uv);

	col.xyz *= 10.0 * col.a;

	gl_FragColor = vec4(col.xyz, 1.);
}
