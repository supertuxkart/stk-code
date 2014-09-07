#ifdef GL_ARB_bindless_texture
layout(bindless_sampler) uniform sampler2D Albedo;
layout(bindless_sampler) uniform sampler2D Detail;
#else
uniform sampler2D Albedo;
uniform sampler2D Detail;
#endif

#if __VERSION__ >= 130
in vec2 uv;
in vec2 uv_bis;
out vec4 FragColor;
#else
varying vec2 uv;
varying vec2 uv_bis;
#define FragColor gl_FragColor
#endif

vec3 getLightFactor(float specMapValue);

void main(void)
{
    vec4 color = texture(Albedo, uv);
#ifdef GL_ARB_bindless_texture
#ifdef SRGBBindlessFix
    color.xyz = pow(color.xyz, vec3(2.2));
#endif
#endif
    vec4 detail = texture(Detail, uv_bis);
    color *= detail;
    vec3 LightFactor = getLightFactor(1. - color.a);
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
