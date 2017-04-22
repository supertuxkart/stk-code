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
    vec3 Frag_normal = normalize(cross(Frag_tangent, bitangent));
    vec3 Frag_bitangent = cross(Frag_normal, Frag_tangent);

    vec3 FragmentNormal = TS_normal.x * Frag_tangent + TS_normal.y * Frag_bitangent - TS_normal.z * Frag_normal;
    EncodedNormal.xy = 0.5 * EncodeNormal(normalize(FragmentNormal)) + 0.5;
    EncodedNormal.z = gloss;
}
