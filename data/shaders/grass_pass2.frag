uniform sampler2D Albedo;
uniform vec3 SunDir;
uniform mat4 invproj;
uniform sampler2D dtex;
uniform vec2 screen;

in vec3 nor;
in vec2 uv;
out vec4 FragColor;

vec3 getLightFactor(float specMapValue);

void main(void)
{
    vec2 texc = gl_FragCoord.xy / screen;
    float z = texture(dtex, texc).x;

    vec4 xpos = 2.0 * vec4(texc, z, 1.0) - 1.0f;
    xpos = invproj * xpos;
    xpos /= xpos.w;
    vec3 eyedir = normalize(xpos.xyz);

    // Inspired from http://http.developer.nvidia.com/GPUGems3/gpugems3_ch16.html
    float fEdotL = max(0., dot(SunDir, eyedir));
    float fPowEdotL = pow(fEdotL, 4.);

    float fLdotNBack  = max(0., - dot(nor, SunDir) * 0.6 + 0.4);
    float scattering = mix(fPowEdotL, fLdotNBack, .5);

    vec4 color = texture(Albedo, uv);
    if (color.a < 0.5) discard;
    vec3 LightFactor = (scattering * 0.3) + getLightFactor(1.);
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
