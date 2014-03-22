uniform sampler2D tex;
uniform sampler2D dtex;
uniform vec3 inlevel;
uniform vec2 outlevel;
uniform mat4 invprojm;

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

    float curdepth = texture(dtex, uv).x;
    vec4 FragPos = invprojm * (2.0 * vec4(uv, curdepth, 1.0f) - 1.0f);
    FragPos /= FragPos.w;
    float depth = clamp(FragPos.z / 180, 0, 1);
    depth = (1 - depth);

    // Compute the vignette
    vec2 inside = uv - 0.5;
    float vignette = 1 - dot(inside, inside);    
    vignette = clamp(pow(vignette, 0.8), 0., 1.);
    vignette = clamp(vignette + vignette - 0.5, 0., 1.15);

	float inBlack = inlevel.x;
	float inWhite = inlevel.z;
	float inGamma = inlevel.y;

	float outBlack = outlevel.x;
	float outWhite = outlevel.y;

	vec3 colSat = (pow(((col.rgb * 255.0) - inBlack) / (inWhite - inBlack),
                vec3(1.0 / inGamma)) * (outWhite - outBlack) + outBlack) / 255.0;

    vec3 colFinal = colSat * depth + col.rgb * (1 - depth);
  
	FragColor = vec4(colFinal * vignette, 1.0);
    //FragColor = vec4(vec3(depth), 1.0);
}
