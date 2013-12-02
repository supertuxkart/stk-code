uniform sampler2D tex;
uniform float low;

void main()
{
	vec3 weights = vec3(0.2126, 0.7152, 0.0722); // ITU-R BT. 709
	vec3 col = texture2D(tex, gl_TexCoord[0].xy).xyz;
	float luma = dot(weights, col);

	col *= smoothstep(low, 0.9, luma);

	gl_FragColor = vec4(col, 1.0);
}
