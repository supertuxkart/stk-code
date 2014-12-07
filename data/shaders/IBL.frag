uniform sampler2D ntex;
uniform sampler2D dtex;

out vec4 Diff;
out vec4 Spec;

vec3 DecodeNormal(vec2 n);
vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);
vec3 DiffuseIBL(vec3 normal);
vec3 SpecularIBL(vec3 normal, vec3 V, float roughness);

void main(void)
{
    vec2 uv = gl_FragCoord.xy / screen;
    vec3 normal = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));

    Diff = vec4(0.25 * DiffuseIBL(normal), 1.);

    float z = texture(dtex, uv).x;

    vec4 xpos = getPosFromUVDepth(vec3(uv, z), InverseProjectionMatrix);
    vec3 eyedir = -normalize(xpos.xyz);
    float specval = texture(ntex, uv).z;

    Spec = vec4(SpecularIBL(normal, eyedir, specval), 1.);
}
