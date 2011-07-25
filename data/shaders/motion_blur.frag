// motion_blur.frag

uniform float boost_amount;	// should be in the range [0.0, 1.0]
uniform sampler2D color_buffer;

// The blur direction points to the following center (we work in [0, 1]x[0, 1] coordinates):
#define BLUR_DIR_CENTER vec2(0.5, 0.7)

// There is a mask around the character so that it doesn't get blurred
#define BLUR_MASK_CENTER vec2(0.5, 0.2)
#define BLUR_MASK_RADIUS 0.15

// Final scaling factor
#define BLUR_SCALE 0.2

// Number of samples used for blurring
#define NB_SAMPLES 12

void main()
{
	vec2 texcoords = gl_TexCoord[0].st;
	
	// Sample the color buffer
	vec3 color = texture2D(color_buffer, texcoords).rgb;
	
	// Compute the blur direction.
	// IMPORTANT: we don't normalize it so that it avoids a glitch around BLUR_DIR_CENTER,
	// plus it naturally scales the motion blur in a cool way :)
	vec2 blur_dir = BLUR_DIR_CENTER - texcoords;
	
	// Compute the blurring factor:
	// - apply the mask
	float blur_factor = max(0.0, length(texcoords - BLUR_MASK_CENTER) - BLUR_MASK_RADIUS);
	
	// - avoid blurring the top of the screen
	blur_factor *= (1.0-texcoords.t);
	
	// - apply the boost amount
	blur_factor *= boost_amount;
	
	// - apply a final scaling factor
	blur_factor *= BLUR_SCALE;
	
	// Scale the blur direction
	blur_dir *= blur_factor;
	
	// Compute the blur
	vec2 inc_vec = blur_dir / vec2(NB_SAMPLES);
	vec2 blur_texcoords = texcoords + inc_vec;
	for(int i=1 ; i < NB_SAMPLES ; i++)
	{
		color += texture2D(color_buffer, blur_texcoords).rgb;
		blur_texcoords += inc_vec;
	}
	color /= vec3(NB_SAMPLES);
	gl_FragColor = vec4(color, 1.0);

	// Keep this commented line for debugging:
//	gl_FragColor = vec4(blur_factor, blur_factor, blur_factor, 0.0);
}
