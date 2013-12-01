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

	
	col = (pow(((col * 255.0) - inBlack) / (inWhite - inBlack),
                (1.0 / inGamma)) * (outWhite - outBlack) + outBlack) / 255.0;
  
	gl_FragColor = vec4(col.rgb, 1.0);
}
