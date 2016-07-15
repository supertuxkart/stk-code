#ifdef Use_Bindless_Texture
layout(bindless_sampler) uniform sampler2D Albedo;
layout(bindless_sampler) uniform sampler2D dtex;
layout(bindless_sampler) uniform sampler2D SpecMap;
#else
uniform sampler2D Albedo;
uniform sampler2D dtex;
uniform sampler2D SpecMap;
#endif

in vec3 nor;
in vec2 uv;
out vec4 FragColor;

#stk_include "utils/getLightFactor.frag"

void main(void)
{
    vec4 color = texture(Albedo, uv);
#ifdef Use_Bindless_Texture
#ifdef SRGBBindlessFix
    color.xyz = pow(color.xyz, vec3(2.2));
#endif
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
    float specmap = texture(SpecMap, uv).g;

    vec3 LightFactor = color.xyz * (scattering * 0.3) + getLightFactor(color.xyz, vec3(1.), specmap, 0.);
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
