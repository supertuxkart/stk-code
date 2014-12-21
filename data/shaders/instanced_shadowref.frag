#ifndef Use_Bindless_Texture
uniform sampler2D tex;
#endif

#ifdef Use_Bindless_Texture
flat in uvec2 handle;
#endif
in vec2 uv;
in vec4 color;
out vec4 FragColor;

void main(void)
{
#ifdef Use_Bindless_Texture
    vec4 col = texture(sampler2D(handle), uv);
#else
    vec4 col = texture(tex, uv);
#endif
    if (col.a < 0.5) discard;
    FragColor = vec4(exp(32. * (2. * gl_FragCoord.z - 1.) / gl_FragCoord.w));
}
