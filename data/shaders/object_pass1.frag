#if __VERSION__ >= 130
in vec3 nor;
out vec2 EncodedNormal;
#else
varying vec3 nor;
#define EncodedNormal gl_FragColor.xy
#endif

vec2 EncodeNormal(vec3 n);

void main(void)
{
	EncodedNormal = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
}
