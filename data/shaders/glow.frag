uniform sampler2D tex;
uniform vec2 res;

void main()
{
	vec2 coords = gl_FragCoord.xy / res;

	vec4 col = texture2D(tex, coords);
	float alpha = col.a;

	if (alpha < 0.04) discard;
    
	col *= vec4(vec3(4.0), 1.5);
    col.a *= 0.6;
    
	gl_FragColor = col;
}
