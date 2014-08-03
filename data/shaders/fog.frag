uniform sampler2D tex;

uniform float fogmax;
uniform float startH;
uniform float endH;
uniform float start;
uniform float end;
uniform vec3 col;

out vec4 FragColor;


vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

void main()
{
    vec2 uv = gl_FragCoord.xy / screen;
	float z = texture(tex, uv).x;
	vec4 xpos = getPosFromUVDepth(vec3(uv, z), InverseProjectionMatrix);

	float dist = length(xpos.xyz);
	float fog = smoothstep(start, end, dist);

	fog = min(fog, fogmax);

	FragColor = vec4(col, fog);
}
