uniform samplerCube tex;
uniform samplerBuffer samples;
uniform float ViewportSize;

uniform mat4 PermutationMatrix;

out vec4 FragColor;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / ViewportSize;
    vec3 RayDir = 2. * vec3(uv, 1.) - 1.;
    RayDir = normalize((PermutationMatrix * vec4(RayDir, 0.)).xyz);

    vec4 FinalColor = vec4(0.);
    vec3 up = (RayDir.y < .99) ? vec3(0., 1., 0.) : vec3(0., 0., 1.);
    vec3 Tangent = normalize(cross(up, RayDir));
    vec3 Bitangent = cross(RayDir, Tangent);
    float weight = 0.;

    for (int i = 0; i < 1024; i++)
    {
        float Theta = texelFetch(samples, i).r;
        float Phi = texelFetch(samples, i).g;

        vec3 L = cos(Theta) * RayDir + sin(Theta) * cos(Phi) * Tangent + sin(Theta) * sin(Phi) * Bitangent;
        float NdotL = clamp(dot(RayDir, L), 0., 1.);
        FinalColor += textureLod(tex, L, 0.) * NdotL;
        weight += NdotL;
    }

    FragColor = FinalColor / weight;
}
