#ifndef GL_ARB_bindless_texture
uniform sampler2D Albedo;
uniform sampler2D Detail;
uniform sampler2D SpecMap;
#endif

#ifdef GL_ARB_bindless_texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
flat in sampler2D thirdhandle;
#endif
in vec2 uv;
in vec2 uv_bis;
out vec4 FragColor;

vec3 getLightFactor(vec3 diffuseMatColor, vec3 specularMatColor, float specMapValue);

void main(void)
{
#ifdef GL_ARB_bindless_texture
    vec4 color = texture(handle, uv);
    float specmap = texture(secondhandle, uv).g;
#ifdef SRGBBindlessFix
    color.xyz = pow(color.xyz, vec3(2.2));
#endif
    vec4 detail = texture(thirdhandle, uv_bis);
#else
    vec4 color = texture(Albedo, uv);
    vec4 detail = texture(Detail, uv_bis);
    float specmap = texture(SpecMap, uv).g;
#endif
    color *= detail;
    FragColor = vec4(getLightFactor(color.xyz, vec3(1.), specmap), 1.);
}
