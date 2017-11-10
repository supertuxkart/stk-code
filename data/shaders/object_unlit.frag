#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D tex;
#else
uniform sampler2D tex;
#endif

in vec2 uv;
in vec4 color;
out vec4 FragColor;

void main(void)
{
    vec4 col = texture(tex, uv);
#ifdef Use_Bindless_Texture
#ifdef SRGBBindlessFix
    col.xyz = pow(col.xyz, vec3(2.2));
#endif
#endif
    if (col.a < 0.5) discard;
    
#if defined(sRGB_Framebuffer_Usable) || defined(Advanced_Lighting_Enabled)
    col.xyz *= pow(color.xyz, vec3(2.2));
#else
    col.xyz *= color.xyz;
#endif

    FragColor = vec4(col.xyz, 1.);
}
