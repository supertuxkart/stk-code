uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform sampler2D SSAO;
uniform vec2 screen;
uniform vec3 ambient;

vec3 getLightFactor(float specMapValue)
{
    vec2 tc = gl_FragCoord.xy / screen;
    vec3 DiffuseComponent = texture(DiffuseMap, tc).xyz;
    vec3 SpecularComponent = texture(SpecularMap, tc).xyz;
    float ao = texture(SSAO, tc).x;
    vec3 tmp = ao * ambient + DiffuseComponent + SpecularComponent * specMapValue;
    return tmp * (0.4 + ao*0.6);
}