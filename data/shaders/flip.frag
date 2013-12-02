uniform sampler2D tex;

void main()
{
	vec2 texc = gl_TexCoord[0].xy;
	texc.y = 1.0 - texc.y;


	vec4 col = texture2D(tex, texc);
	
	//col = col * 256;
	//col = (1 - col);
	
	col = col / (1 - col);
	
	gl_FragColor = vec4(col.rgb, 1.0);
}
