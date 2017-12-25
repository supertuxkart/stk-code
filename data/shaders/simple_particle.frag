uniform sampler2D tex;
uniform sampler2D dtex;

in vec2 tc;
in vec4 pc;
flat in float billboard_mix;
out vec4 FragColor;

#stk_include "utils/getPosFromUVDepth.frag"

void main(void)
{
    vec2 xy = gl_FragCoord.xy / u_screen;
    float FragZ = gl_FragCoord.z;
    vec4 FragmentPos = getPosFromUVDepth(vec3(xy, FragZ), u_inverse_projection_matrix);
    float EnvZ = texture(dtex, xy).x;
    vec4 EnvPos = getPosFromUVDepth(vec3(xy, EnvZ), u_inverse_projection_matrix);
    float alpha = clamp((EnvPos.z - FragmentPos.z) * 0.3, 0., 1.);
    float billboard_alpha = mix(1.0, texture(tex, tc).a, billboard_mix);
    FragColor = texture(tex, tc) * billboard_alpha * pc * alpha;
}
