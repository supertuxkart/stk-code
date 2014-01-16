#version 130
uniform sampler2D Albedo;
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform vec2 screen;
uniform vec3 ambient;
in vec2 uv;

void main(void)
{
    vec2 tc = gl_FragCoord.xy / screen;
    vec4 color = texture2D(Albedo, uv);
    vec3 DiffuseComponent = texture2D(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture2D(SpecularMap, tc).xyz;
    vec3 LightFactor = ambient + DiffuseComponent + SpecularComponent * (1. - color.a);
    gl_FragColor = vec4(color.xyz * LightFactor, 1.);
}
