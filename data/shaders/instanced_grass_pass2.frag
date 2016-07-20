#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D dtex;
#else
uniform sampler2D Albedo;
uniform sampler2D SpecMap;
uniform sampler2D dtex;
#endif

#ifdef Use_Bindless_Texture
flat in sampler2D handle;
flat in sampler2D secondhandle;
#endif
in vec3 nor;
in vec2 uv;
out vec4 FragColor;

#stk_include "utils/getLightFactor.frag"

void main(void)
{
#ifdef Use_Bindless_Texture
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
    vec3 L = normalize((transpose(InverseViewMatrix) * vec4(sun_direction, 0.)).xyz);
    float fEdotL = clamp(dot(L, eyedir), 0., 1.);
    float fPowEdotL = pow(fEdotL, 4.);

    float fLdotNBack  = max(0., - dot(nor, L) * 0.6 + 0.4);
    float scattering = mix(fPowEdotL, fLdotNBack, .5);


    vec3 LightFactor = color.xyz * (scattering * 0.1) + getLightFactor(color.xyz, vec3(1.), specmap, 0.);
    FragColor = vec4(LightFactor, 1.);
}
