#version 130
uniform sampler2D tex;

uniform float fogmax;
uniform float startH;
uniform float endH;
uniform float start;
uniform float end;
uniform vec3 col;
uniform mat4 ipvmat;

in vec2 uv;
out vec4 FragColor;

void main()
{
	float z = texture(tex, uv).x;

	vec3 tmp = vec3(uv, z);
	tmp = tmp * 2.0 - 1.0;

	vec4 xpos = vec4(tmp, 1.0);
	xpos = ipvmat * xpos;
	xpos.xyz /= xpos.w;

	float dist = length(xpos.xyz);
	float fog = smoothstep(start, end, dist);

	fog = min(fog, fogmax);

	FragColor = vec4(col, fog);
}
