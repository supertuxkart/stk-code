#version 130
uniform sampler2D tex;
uniform vec3 inlevel;
uniform vec2 outlevel;

void main()
{
	vec2 texc = gl_TexCoord[0].xy;
	texc.y = 1.0 - texc.y;


	vec4 col = texture2D(tex, texc);

	//col = col / (1 - col);

	float inBlack = inlevel.x;
	float inWhite = inlevel.z;
	float inGamma = inlevel.y;

	float outBlack = outlevel.x;
	float outWhite = outlevel.y;

	col.rgb = (pow(((col.rgb * 255.0) - inBlack) / (inWhite - inBlack),
                vec3(1.0 / inGamma)) * (outWhite - outBlack) + outBlack) / 255.0;
  
	gl_FragColor = vec4(col.rgb, 1.0);
}
