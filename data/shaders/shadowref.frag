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
    if (col.a < 0.5) discard;
    FragColor = vec4(exp(32. * (2. * gl_FragCoord.z - 1.) / gl_FragCoord.w));
}
