#ifndef GL_ARB_bindless_texture
uniform sampler2D Albedo;
uniform sampler2D SpecMap;
#endif

#ifdef GL_ARB_bindless_texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
#endif
in vec2 uv;
in vec4 color;
out vec4 FragColor;

vec3 getLightFactor(vec3 diffuseMatColor, vec3 specularMatColor, float specMapValue);

void main(void)
{
#ifdef GL_ARB_bindless_texture
    vec4 col = texture(handle, uv);
    float specmap = texture(secondhandle, uv).g;
#ifdef SRGBBindlessFix
    col.xyz = pow(col.xyz, vec3(2.2));
#endif
#else
    vec4 col = texture(Albedo, uv);
    float specmap = texture(SpecMap, uv).g;
#endif
    col.xyz *= pow(color.xyz, vec3(2.2));
    FragColor = vec4(getLightFactor(col.xyz, vec3(1.), specmap), 1.);
}
