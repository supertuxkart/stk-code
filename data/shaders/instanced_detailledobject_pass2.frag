#ifndef GL_ARB_bindless_texture
uniform sampler2D Albedo;
uniform sampler2D Detail;
#endif

#ifdef GL_ARB_bindless_texture
flat in sampler2D handle;
flat in sampler2D thirdhandle;
#endif
in vec2 uv;
in vec2 uv_bis;
out vec4 FragColor;

vec3 getLightFactor(float specMapValue);

void main(void)
{
#ifdef GL_ARB_bindless_texture
    vec4 color = texture(handle, uv);
#ifdef SRGBBindlessFix
    color.xyz = pow(color.xyz, vec3(2.2));
#endif
    vec4 detail = texture(thirdhandle, uv_bis);
#else
    vec4 color = texture(Albedo, uv);
    vec4 detail = texture(Detail, uv_bis);
#endif
    color *= detail;
    vec3 LightFactor = getLightFactor(1. - color.a);
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
