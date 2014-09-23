#ifndef GL_ARB_bindless_texture
uniform sampler2D tex;
uniform sampler2D glosstex;
#endif

#ifdef GL_ARB_bindless_texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
#endif
in vec3 nor;
in vec2 uv;
out vec3 EncodedNormal;

vec2 EncodeNormal(vec3 n);

void main() {
#ifdef GL_ARB_bindless_texture
    vec4 col = texture(handle, uv);
    float glossmap = texture(secondhandle, uv).x;
#else
    vec4 col = texture(tex, uv);
    float glossmap = texture(glosstex, uv).x;
#endif
    if (col.a < 0.5)
        discard;
    EncodedNormal.xy = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
    EncodedNormal.z = exp2(10. * glossmap + 1.);
}

