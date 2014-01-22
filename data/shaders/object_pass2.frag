#version 130
uniform sampler2D Albedo;
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform sampler2D SSAO;
uniform vec2 screen;
uniform vec3 ambient;
in vec2 uv;
out vec4 FragColor;

void main(void)
{
    vec2 tc = gl_FragCoord.xy / screen;
    vec4 color = texture(Albedo, uv);
    vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
	float ao = texture(SSAO, tc).x;
    vec3 LightFactor = ao * ambient + DiffuseComponent + SpecularComponent * (1. - color.a);

    FragColor = vec4(color.xyz * LightFactor * ao, 1.);
}
