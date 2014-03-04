uniform sampler2D Albedo;
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform sampler2D SSAO;
uniform vec2 screen;
uniform vec3 ambient;
uniform sampler2D caustictex;
uniform vec2 dir;
uniform vec2 dir2;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif

void main()
{
    vec2 tc = gl_FragCoord.xy / screen;
    vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
    vec4 color = texture(Albedo, uv);
    float ao = texture(SSAO, tc).x;

    float caustic = texture(caustictex, uv + dir).x;
    float caustic2 = texture(caustictex, (uv.yx + dir2 * vec2(-0.6, 0.3)) * vec2(0.6)).x;

    vec3 LightFactor = ao * ambient + DiffuseComponent + SpecularComponent + caustic * caustic2 * 10;
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
