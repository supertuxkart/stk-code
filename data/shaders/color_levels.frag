uniform sampler2D tex;
uniform vec3 inlevel;
uniform vec2 outlevel;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif



void main()
{
	vec4 col = texture(tex, uv);

    // Compute the vignette
    vec2 inside = uv - 0.5;
    float vignette = 1 - dot(inside, inside);    
    vignette = clamp(pow(vignette, 0.8), 0, 1);
    vignette = vignette + vignette - 0.5;

	float inBlack = inlevel.x;
	float inWhite = inlevel.z;
	float inGamma = inlevel.y;

	float outBlack = outlevel.x;
	float outWhite = outlevel.y;

	col.rgb = (pow(((col.rgb * 255.0) - inBlack) / (inWhite - inBlack),
                vec3(1.0 / inGamma)) * (outWhite - outBlack) + outBlack) / 255.0;
  
	FragColor = vec4(col.rgb * vignette, 1.0);
}
