//#define DOF_ENABLED

#ifdef DOF_ENABLED

uniform sampler2D tex;
uniform sampler2D dtex;
uniform vec3 inlevel;
uniform vec2 outlevel;
uniform mat4 invprojm;

in vec2 uv;
out vec4 FragColor;

#define PI  3.14159265

/* ---------------------------------------------------------------------------------------------- */
// TEST
/* ---------------------------------------------------------------------------------------------- */

float width = 1920; //texture width
float height = 1080; //texture height

vec2 texel = vec2(1.0/width,1.0/height);

//------------------------------------------
//user variables

int samples = 3; //samples on the first ring
int rings = 5; //ring count

bool autofocus = false; //use autofocus in shader? disable if you use external focalDepth value
float focalDepth = 0.1;
vec2 focus = vec2(0.5,0.5); // autofocus point on screen (0.0,0.0 - left lower corner, 1.0,1.0 - upper right)
float range = 150.0; //focal range
float maxblur = 1.25; //clamp value of max blur

float threshold = 0.9; //highlight threshold;
float gain = 10.0; //highlight gain;

float bias = 0.4; //bokeh edge bias
float fringe = 0.5; //bokeh chromatic aberration/fringing

bool noise = true; //use noise instead of pattern for sample dithering
float namount = 0.0001; //dither amount

bool depthblur = false; //blur the depth buffer?
float dbsize = 2.0; //depthblursize



/* ---------------------------------------------------------------------------------------------- */
// Function
/* ---------------------------------------------------------------------------------------------- */

vec3 color(vec2 coords,float blur) //processing the sample
{
	vec3 col = vec3(0.0);
	
	col.r = texture2D(tex,coords + vec2(0.0,1.0)*texel*fringe*blur).r;
	col.g = texture2D(tex,coords + vec2(-0.866,-0.5)*texel*fringe*blur).g;
	col.b = texture2D(tex,coords + vec2(0.866,-0.5)*texel*fringe*blur).b;
	
	vec3 lumcoeff = vec3(0.299,0.587,0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum-threshold)*gain, 0.0);
	return col+mix(vec3(0.0),col,thresh*blur);
}

vec2 rand(in vec2 coord) //generating noise/pattern texture for dithering
{
	float noiseX = ((fract(1.0-coord.s*(width/2.0))*0.25)+(fract(coord.t*(height/2.0))*0.75))*2.0-1.0;
	float noiseY = ((fract(1.0-coord.s*(width/2.0))*0.75)+(fract(coord.t*(height/2.0))*0.25))*2.0-1.0;
	
	if (noise)
	{
		noiseX = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233))) * 43758.5453),0.0,1.0)*2.0-1.0;
		noiseY = clamp(fract(sin(dot(coord ,vec2(12.9898,78.233)*2.0)) * 43758.5453),0.0,1.0)*2.0-1.0;
	}
	return vec2(noiseX,noiseY);
}






void main() 
{
	
	//float depth = texture2D(dtex,uv).x;
	
	float curdepth = texture(dtex, uv).x;
	vec4 FragPos = invprojm * (2.0f * vec4(uv, curdepth, 1.0f) - 1.0f);
	FragPos /= FragPos.w;
	
	float depth = clamp((FragPos.z/180), 0., 1.);
	
	
	float blur = 0.0;
	
	
	blur = clamp((abs(depth - focalDepth)/range)*100.0,-maxblur,maxblur);

	vec2 noise = rand(uv)*namount*blur;
	
	float w = (1.0/width)*blur+noise.x;
	float h = (1.0/height)*blur+noise.y;
	
	vec3 col = texture2D(tex, uv).rgb;
	vec3 colDof = col;
	float s = 1.0;
	
	int ringsamples;
	
	for (int i = 1; i <= rings; i += 1)
	{   
		ringsamples = i * samples;
		 
		for (int j = 0 ; j < ringsamples ; j += 1)   
		{
			float step = PI*2.0 / float(ringsamples);
			float pw = (cos(float(j)*step)*float(i));
			float ph = (sin(float(j)*step)*float(i));
			float p = 1.0;

			colDof += color(uv + vec2(pw*w,ph*h),blur)*mix(1.0,(float(i))/(float(rings)),bias)*p;  
			s += 1.0*mix(1.0,(float(i))/(float(rings)),bias)*p;   
		}
	}
	colDof /= s;
	
	// get color correction values
	float inBlack = inlevel.x;
	float inWhite = inlevel.z;
	float inGamma = inlevel.y;

	float outBlack = outlevel.x;
	float outWhite = outlevel.y;
	
	vec3 colOut = (pow(((col.rgb * 255.0) - inBlack) / (inWhite - inBlack),
                vec3(1.0 / inGamma)) * (outWhite - outBlack) + outBlack) / 255.0;
	
	depth  = (1 - depth);
	vec3 final = colOut * depth + colDof.rgb * (1 - depth);

  vec2 inTex = uv - 0.5;
	float vignette  = 1 - dot(inTex, inTex);

  vignette = clamp(pow(vignette, 0.8), 0., 1.) ;
  vignette = vignette + vignette - 0.5;
  final.rgb *= clamp(vignette, 0., 1.15);

	FragColor.rgb = final;
	FragColor.a = 1.0;
}



/*
void main()
{
	vec2 texc = uv;
	//texc.y = 1.0 - texc.y;


	vec4 col = texture(tex, texc);
	float curdepth = texture(dtex, uv).x;
	
	vec2 inTex = uv - 0.5;
	float vignette  = 1 - dot(inTex, inTex);
	
	vec4 FragPos = invprojm * (2.0f * vec4(uv, curdepth, 1.0f) - 1.0f);
	FragPos /= FragPos.w;

	//col = col / (1 - col);

	float inBlack = inlevel.x;
	float inWhite = inlevel.z;
	float inGamma = inlevel.y;

	float outBlack = outlevel.x;
	float outWhite = outlevel.y;

  float depth1 = clamp((FragPos.z/180), 0., 1.);

	vec3 colOut = (pow(((col.rgb * 255.0) - inBlack) / (inWhite - inBlack),
                vec3(1.0 / inGamma)) * (outWhite - outBlack) + outBlack) / 255.0;
  
  depth1  = (1 - depth1);
  vec3 final = colOut * depth1 + col.rgb * (1 - depth1);
  
  vignette = clamp(pow(vignette, 0.8), 0., 1.) ;
  vignette = vignette + vignette - 0.5;
  final.rgb *= clamp(vignette, 0., 1.15);
  FragColor = vec4(final, 1.0);


}*/

#else


uniform sampler2D tex;
uniform sampler2D dtex;
uniform vec3 inlevel;
uniform vec2 outlevel;
uniform mat4 invprojm;

in vec2 uv;
out vec4 FragColor;

void main()
{
    vec4 col = texture(tex, uv);

    float curdepth = texture(dtex, uv).x;
    vec4 FragPos = invprojm * (2.0 * vec4(uv, curdepth, 1.0f) - 1.0f);
    FragPos /= FragPos.w;
    float depth = clamp(FragPos.z / 180, 0., 1.);
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
#endif