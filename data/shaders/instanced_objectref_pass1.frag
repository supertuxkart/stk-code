#ifndef GL_ARB_bindless_texture
uniform sampler2D tex;
#endif

#ifdef GL_ARB_bindless_texture
flat in sampler2D handle;
#endif
in vec3 nor;
in vec2 uv;
out vec3 EncodedNormal;

vec2 EncodeNormal(vec3 n);

void main() {
#ifdef GL_ARB_bindless_texture
    vec4 col = texture(handle, uv);
#else
    vec4 col = texture(tex, uv);
#endif
    if (col.a < 0.5)
        discard;
    EncodedNormal.xy = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
    EncodedNormal.z = 1.;
}

