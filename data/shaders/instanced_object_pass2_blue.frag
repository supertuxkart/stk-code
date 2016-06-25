#ifndef Use_Bindless_Texture
uniform sampler2D Albedo;
uniform sampler2D SpecMap;
#endif

#ifdef Use_Bindless_Texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
#endif
in vec2 uv;
in vec4 color;
out vec4 FragColor;

vec3 getLightFactor(vec3 diffuseMatColor, vec3 specularMatColor, float specMapValue, float emitMapValue);
vec3 rgbToHsv(vec3 c);
vec3 hsvToRgb(vec3 c);

void main(void)
{
#ifdef Use_Bindless_Texture
    vec4 col = texture(handle, uv);
    float specmap = texture(secondhandle, uv).g;
    float emitmap = texture(secondhandle, uv).b;
#ifdef SRGBBindlessFix
    col.xyz = pow(col.xyz, vec3(2.2));
#endif
#else
    vec4 col = texture(Albedo, uv);
    float specmap = texture(SpecMap, uv).g;
    float emitmap = texture(SpecMap, uv).b;
#endif

    vec3 old_color = rgbToHsv(col.rgb);
    vec3 new_color = hsvToRgb(vec3(0.1,0.5,old_color.z));
    col = vec4(new_color.b, new_color.g, new_color.r, col.a);
    col.xyz *= pow(color.xyz, vec3(2.2));

    FragColor = vec4(getLightFactor(col.xyz, vec3(1.), specmap, emitmap) , 1.);
}
