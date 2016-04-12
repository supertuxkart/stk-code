#ifndef Use_Bindless_Texture
uniform sampler2D tex;
#endif

in vec2 uv;
in vec3 nor;
in vec4 color;
#ifdef Use_Bindless_Texture
flat in uvec2 handle;
#endif
layout (location = 0) out vec3 RSMColor;
layout (location = 1) out vec3 RSMNormals;

void main()
{
#ifdef Use_Bindless_Texture
    vec4 col = texture(sampler2D(handle), uv);
#ifdef SRGBBindlessFix
    col.xyz = pow(col.xyz, vec3(2.2));
#endif
#else
    vec4 col = texture(tex, uv);
#endif
    if (col.a < .5) discard;
    RSMColor = col.xyz * color.rgb;
    RSMNormals = .5 * normalize(nor) + .5;
}
