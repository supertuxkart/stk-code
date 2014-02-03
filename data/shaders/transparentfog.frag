#version 130
uniform sampler2D tex;

uniform float fogmax;
uniform float startH;
uniform float endH;
uniform float start;
uniform float end;
uniform vec3 col;
uniform mat4 ipvmat;
uniform vec2 screen;

in vec2 uv;
out vec4 FragColor;

void main()
{
	vec4 color = texture(tex, uv);
	vec3 tmp = vec3(gl_FragCoord.xy / screen, gl_FragCoord.z);
	tmp = 2. * tmp - 1.;

	vec4 xpos = vec4(tmp, 1.0);
	xpos = ipvmat * xpos;
	xpos.xyz /= xpos.w;

	float dist = length(xpos.xyz);
	float fog = smoothstep(start, end, dist);

	fog = min(fog, fogmax);

	FragColor = vec4(vec4(col, 1.) * fog + color *(1. - fog));
}
