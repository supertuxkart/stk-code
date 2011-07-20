// motion_blur.frag

uniform float boost_amount;
uniform sampler2D color_buffer;

#define BLUR_CENTER vec2(0.5, 0.3)
#define BLUR_SCALE 0.1
#define NB_SAMPLES 12

void main()
{
	vec2 texcoords = gl_TexCoord[0].st;
	vec3 color = texture2D(color_buffer, texcoords);
	vec2 blur_dir = -BLUR_SCALE * (texcoords - BLUR_CENTER);
	
	// Avoid blurring the top of the screen
	float blur_factor = (1.0-texcoords.t);
	blur_dir *= blur_factor;
	
	vec2 inc_vec = blur_dir / NB_SAMPLES;
	vec2 blur_texcoords = texcoords + inc_vec;
	for(int i=1 ; i < NB_SAMPLES ; i++)
	{
		color += texture2D(color_buffer, blur_texcoords);
		blur_texcoords += inc_vec;
	}
	color /= NB_SAMPLES;
	
	gl_FragColor = vec4(color, 0.0);
}
