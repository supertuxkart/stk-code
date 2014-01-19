#version 130
uniform sampler2D tex;

in vec2 uv;
in vec4 col;

void main()
{
	vec4 res = texture(tex, uv);
	gl_FragColor = vec4(res.xyz * col.xyz, res.a);
}
