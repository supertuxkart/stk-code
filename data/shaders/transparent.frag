#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D tex;
#else
uniform sampler2D tex;
uniform float custom_alpha;
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
    Color.a *= color.a;
    // Premultiply alpha
    FragColor = vec4(Color.rgb * (Color.a * custom_alpha), Color.a * custom_alpha);
}
