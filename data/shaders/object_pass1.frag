#ifdef GL_ARB_bindless_texture
layout(bindless_sampler) uniform sampler2D tex;
#else
uniform sampler2D tex;
#endif

#if __VERSION__ >= 130
in vec3 nor;
in vec2 uv;
out vec3 EncodedNormal;
#else
varying vec3 nor;
#define EncodedNormal gl_FragColor.xy
#endif

vec2 EncodeNormal(vec3 n);

void main(void)
{
	float glossmap = texture(tex, uv).x;
	EncodedNormal.xy = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
	EncodedNormal.z = exp2(10. * glossmap + 1.);
}
