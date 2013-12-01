uniform sampler2D tex;

void main()
{
	vec2 texc = gl_TexCoord[0].xy;
	texc.y = 1.0 - texc.y;


	vec4 col = texture2D(tex, texc);

	//col = col / (1 - col);
	

	float inBlack = 0.0;
	float inWhite = 137.0;
	float inGamma = 0.65;

	
	float outWhite = 255.0;
	float outBlack = 0.0;

	
	col.rgb = (pow(((col.rgb * 255.0) - inBlack) / (inWhite - inBlack),
                vec3(1.0 / inGamma)) * (outWhite - outBlack) + outBlack) / 255.0;
  
	gl_FragColor = vec4(col.rgb, 1.0);
}
