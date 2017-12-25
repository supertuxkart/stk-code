#ifndef Use_Bindless_Texture
uniform sampler2D normalMap;
uniform sampler2D glossMap;
#endif

#ifdef Use_Bindless_Texture
flat in sampler2D secondhandle;
flat in sampler2D fourthhandle;
#endif
in vec3 tangent;
in vec3 bitangent;
in vec3 nor;
flat in float bitangent_sign;
in vec2 uv;
out vec3 EncodedNormal;

#stk_include "utils/encode_normal.frag"

void main()
{
    // normal in Tangent Space
#ifdef Use_Bindless_Texture
    vec3 TS_normal = 2.0 * texture(fourthhandle, uv).rgb - 1.0;
    float gloss = texture(secondhandle, uv).x;
#else
    vec3 TS_normal = 2.0 * texture(normalMap, uv).rgb - 1.0;
    float gloss = texture(glossMap, uv).x;
#endif
    // Because of interpolation, we need to renormalize
    vec3 Frag_tangent = normalize(tangent);
    vec3 Frag_normal = normalize(nor);
    vec3 Frag_bitangent = normalize(cross(Frag_normal, tangent) * bitangent_sign);
    vec3 FragmentNormal = mat3(Frag_tangent, Frag_bitangent, Frag_normal) * TS_normal;
    EncodedNormal.xy = 0.5 * EncodeNormal(normalize(FragmentNormal)) + 0.5;
    EncodedNormal.z = gloss;
}
