uniform sampler2D tex;

#if __VERSION__ >= 130
in vec3 nor;
in vec2 uv;
out vec2 EncodedNormal;
#else
varying vec3 nor;
varying vec2 uv;
#define EncodedNormal gl_FragColor.xy
#endif

vec2 EncodeNormal(vec3 n);

void main() {
	vec4 col = texture(tex, uv);
	if (col.a < 0.5)
		discard;
	EncodedNormal = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
}

