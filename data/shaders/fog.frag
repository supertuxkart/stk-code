#version 130
uniform sampler2D tex;

uniform float fogmax;
uniform float startH;
uniform float endH;
uniform float start;
uniform float end;
uniform vec3 col;
uniform vec3 campos;
uniform mat4 ipvmat;

in vec2 uv;
out vec4 FragColor;

void main()
{
	float z = texture(tex, uv).a;

	vec3 tmp = vec3(gl_TexCoord[0].xy, z);
	tmp = tmp * 2.0 - 1.0;

	vec4 xpos = vec4(tmp, 1.0);
	xpos = ipvmat * xpos;
	xpos.xyz /= xpos.w;

	float dist = distance(campos, xpos.xyz);
	float fog = smoothstep(start, end, dist);
	fog *= 1.0 - smoothstep(startH, endH, xpos.y);

	fog = min(fog, fogmax);

	FragColor = vec4(col, fog);
}
