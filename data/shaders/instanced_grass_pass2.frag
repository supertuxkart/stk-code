#ifdef GL_ARB_bindless_texture
layout(bindless_sampler) uniform sampler2D dtex;
#else
uniform sampler2D Albedo;
uniform sampler2D SpecMap;
uniform sampler2D dtex;
#endif

uniform vec3 SunDir;

#ifdef GL_ARB_bindless_texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
#endif
in vec3 nor;
in vec2 uv;
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
#else
    vec4 color = texture(Albedo, uv);
    float specmap = texture(SpecMap, uv).g;
#endif
    if (color.a < 0.5) discard;

    vec2 texc = gl_FragCoord.xy / screen;
    float z = texture(dtex, texc).x;

    vec4 xpos = 2.0 * vec4(texc, z, 1.0) - 1.0f;
    xpos = InverseProjectionMatrix * xpos;
    xpos /= xpos.w;
    vec3 eyedir = normalize(xpos.xyz);

    // Inspired from http://http.developer.nvidia.com/GPUGems3/gpugems3_ch16.html
    float fEdotL = max(0., dot(SunDir, eyedir));
    float fPowEdotL = pow(fEdotL, 4.);

    float fLdotNBack  = max(0., - dot(nor, SunDir) * 0.6 + 0.4);
    float scattering = mix(fPowEdotL, fLdotNBack, .5);


    vec3 LightFactor = color.xyz * (scattering * 0.3) + getLightFactor(color.xyz, vec3(1.), specmap);
    FragColor = vec4(LightFactor, 1.);
}
