uniform sampler2D ntex;
#if defined(GL_ES) && defined(GL_FRAGMENT_PRECISION_HIGH)
uniform highp sampler2D dtex;
#else
uniform sampler2D dtex;
#endif

flat in vec3 center;
flat in float energy;
flat in vec3 col;
flat in float radius;

#ifdef GL_ES
layout (location = 0) out vec4 Diff;
layout (location = 1) out vec4 Spec;
#else
out vec4 Diff;
out vec4 Spec;
#endif

#stk_include "utils/decodeNormal.frag"
#stk_include "utils/SpecularBRDF.frag"
#stk_include "utils/DiffuseBRDF.frag"
#stk_include "utils/getPosFromUVDepth.frag"

void main()
{
    vec2 texc = gl_FragCoord.xy / u_screen;
    float z = texture(dtex, texc).x;
    vec3 norm = (u_view_matrix * vec4(DecodeNormal(texture(ntex, texc).xy), 0)).xyz;
    float roughness = texture(ntex, texc).z;

    vec4 xpos = getPosFromUVDepth(vec3(texc, z), u_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos.xyz);

    vec4 pseudocenter = u_view_matrix * vec4(center.xyz, 1.0);
    pseudocenter /= pseudocenter.w;
    vec3 light_pos = pseudocenter.xyz;
    vec3 light_col = col.xyz;
    float d = distance(light_pos, xpos.xyz);
    float att = energy * 20. / (1. + d * d);
    att *= (radius - d) / radius;
    if (att <= 0.) discard;

    // Light Direction
    vec3 L = (light_pos - xpos.xyz) / d;

    float NdotL = clamp(dot(norm, L), 0., 1.);
    vec3 Specular = SpecularBRDF(norm, eyedir, L, vec3(1.), roughness);
    vec3 Diffuse = DiffuseBRDF(norm, eyedir, L, vec3(1.), roughness);

    Diff = vec4(Diffuse * NdotL * light_col * att, 1.);
    Spec = vec4(Specular * NdotL * light_col * att, 1.);
}