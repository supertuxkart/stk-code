#ifndef Use_Bindless_Texture
uniform sampler2D tex;
#endif

#ifdef Use_Bindless_Texture
flat in sampler2D handle;
#endif

in vec2 uv;
in vec4 color;
out vec4 FragColor;

void main(void)
{
#ifdef Use_Bindless_Texture
    vec4 col = texture(handle, uv);
#ifdef SRGBBindlessFix
    col.xyz = pow(col.xyz, vec3(2.2));
#endif
#else
    vec4 col = texture(tex, uv);
#endif
    if (col.a < 0.5) discard;
    
#if defined(sRGB_Framebuffer_Usable) || defined(Advanced_Lighting_Enabled)
    col.xyz *= pow(color.xyz, vec3(2.2));
#else
    col.xyz *= color.xyz;
#endif
    
    FragColor = vec4(col.xyz, 1.);
}
