#ifndef Use_Bindless_Texture
uniform sampler2D tex;
uniform sampler2D glosstex;
#endif

#ifdef Use_Bindless_Texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
#endif
in vec3 nor;
in vec2 uv;
out vec3 EncodedNormal;

#stk_include "utils/encode_normal.frag"

void main() {
#ifdef Use_Bindless_Texture
    vec4 col = texture(handle, uv);
    float glossmap = texture(secondhandle, uv).x;
#else
    vec4 col = texture(tex, uv);
    float glossmap = texture(glosstex, uv).x;
#endif
    if (col.a < 0.5)
        discard;
    EncodedNormal.xy = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
    EncodedNormal.z = glossmap;
}

