uniform sampler2D ntex;
uniform sampler2D ssao;

#ifdef GL_ES
layout (location = 0) out vec4 Diff;
layout (location = 1) out vec4 Spec;
#else
out vec4 Diff;
out vec4 Spec;
#endif

#stk_include "utils/decodeNormal.frag"
#stk_include "utils/getPosFromUVDepth.frag"
#stk_include "utils/DiffuseIBL.frag"
#stk_include "utils/SpecularIBL.frag"

void main(void)
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    vec3 normal = DecodeNormal(texture(ntex, uv).xy);
    vec3 spec_color = vec3(0.031, 0.106, 0.173);
    float ao = texture(ssao, uv).x;

    Diff = vec4(0.25 * DiffuseIBL(normal) * ao, 1.);
    Spec = vec4(spec_color * ao, 1.);
}
