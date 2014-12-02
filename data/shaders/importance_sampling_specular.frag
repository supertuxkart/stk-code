uniform samplerCube tex;
uniform float samples[2048];
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

    for (int i = 0; i < 1024; i++)
    {
        float Theta = samples[2 * i];
        float Phi = samples[2 * i + 1];

        vec3 sampleDir = cos(Theta) * RayDir + sin(Theta) * cos(Phi) * Tangent + sin(Theta) * sin(Phi) * Bitangent;
        FinalColor += textureLod(tex, sampleDir, 0.);
    }

    FragColor = FinalColor / 1024.;
}
