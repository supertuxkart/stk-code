#ifdef GL_ARB_bindless_texture
layout(bindless_sampler) uniform sampler2D Albedo;
layout(bindless_sampler) uniform sampler2D Detail;
layout(bindless_sampler) uniform sampler2D SpecMap;
#else
uniform sampler2D Albedo;
uniform sampler2D Detail;
uniform sampler2D SpecMap;
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

vec3 getLightFactor(vec3 diffuseMatColor, vec3 specularMatColor, float specMapValue);

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
    float specmap = texture(SpecMap, uv).g;
    FragColor = vec4(getLightFactor(color.xyz, vec3(1.), specmap), 1.);
}
