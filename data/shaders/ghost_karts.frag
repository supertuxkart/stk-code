#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D tex;
#else
uniform sampler2D tex;
#endif

in vec2 uv;
in vec4 color;
out vec4 FragColor;

void main()
{
    vec4 Color = texture(tex, uv);
#ifdef Use_Bindless_Texture
    Color.xyz = pow(Color.xyz, vec3(2.2));
#endif
    Color.xyz *= pow(color.xyz, vec3(2.2));
    FragColor = vec4(Color.rgb * 0.5, 0.5);
}
