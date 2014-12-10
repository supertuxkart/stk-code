uniform sampler2D ntex;
uniform sampler2D dtex;

flat in vec3 center;
flat in float energy;
flat in vec3 col;
flat in float radius;

out vec4 Diffuse;
out vec4 Specular;

vec3 DecodeNormal(vec2 n);
vec3 SpecularBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness);
vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

void main()
{
    vec2 texc = gl_FragCoord.xy / screen;
    float z = texture(dtex, texc).x;
    vec3 norm = normalize(DecodeNormal(2. * texture(ntex, texc).xy - 1.));
    float roughness = texture(ntex, texc).z;

    vec4 xpos = getPosFromUVDepth(vec3(texc, z), InverseProjectionMatrix);
    vec3 eyedir = -normalize(xpos.xyz);

    vec4 pseudocenter = ViewMatrix * vec4(center.xyz, 1.0);
    pseudocenter /= pseudocenter.w;
    vec3 light_pos = pseudocenter.xyz;
    vec3 light_col = col.xyz;
    float d = distance(light_pos, xpos.xyz);
    float att = energy * 20. / (1. + d * d);
    att *= (radius - d) / radius;
    if (att <= 0.) discard;

    // Light Direction
    vec3 L = -normalize(xpos.xyz - light_pos);

    float NdotL = clamp(dot(norm, L), 0., 1.);

    Diffuse = vec4(NdotL * light_col * att, 1.);
    Specular = vec4(SpecularBRDF(norm, eyedir, L, light_col, roughness) * NdotL * att, 1.);
}
