#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D tex;
layout(bindless_sampler) uniform sampler2D glosstex;
#else
uniform sampler2D tex;
uniform sampler2D glosstex;
#endif

in vec3 nor;
in vec2 uv;
out vec3 EncodedNormal;

#stk_include "utils/encode_normal.frag"

void main() {
	vec4 col = texture(tex, uv);
	if (col.a < 0.5)
		discard;
	float glossmap = texture(glosstex, uv).x;
	EncodedNormal.xy = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
	EncodedNormal.z = glossmap;
}

