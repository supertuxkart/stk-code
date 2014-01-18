#version 130
uniform sampler2D tex;

in vec2 uv;

void main()
{
	vec2 coords = uv;

	vec4 col = texture2D(tex, coords);
	float alpha = col.a;

	if (alpha < 0.04 || length(col.xyz) < 0.2) discard;
    
	col *= vec4(vec3(4.0), 1.5);
    col.a *= 0.6;
    
	gl_FragColor = col;
}
