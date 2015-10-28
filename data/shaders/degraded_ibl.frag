uniform sampler2D ntex;

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
    Spec = vec4(0., 0., 0., 1.);
}
