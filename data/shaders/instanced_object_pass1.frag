#ifndef Use_Bindless_Texture
uniform sampler2D glosstex;
#endif

#ifdef Use_Bindless_Texture
flat in sampler2D secondhandle;
#endif
in vec3 nor;
in vec2 uv;
out vec3 EncodedNormal;

vec2 EncodeNormal(vec3 n);

void main(void)
{
#ifdef Use_Bindless_Texture
    float glossmap = texture(secondhandle, uv).x;
#else
    float glossmap = texture(glosstex, uv).x;
#endif
    EncodedNormal.xy = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
    EncodedNormal.z = glossmap;
}
