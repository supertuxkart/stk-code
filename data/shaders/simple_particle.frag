uniform sampler2D tex;
uniform sampler2D dtex;
uniform float billboard;

in vec2 tc;
in vec4 pc;
out vec4 FragColor;

#stk_include "utils/getPosFromUVDepth.frag"

void main(void)
{
    vec4 color = texture(tex, tc) * pc;
#if defined(Advanced_Lighting_Enabled)
    vec2 xy = gl_FragCoord.xy / u_screen;
    float FragZ = gl_FragCoord.z;
    vec4 FragmentPos = getPosFromUVDepth(vec3(xy, FragZ), u_inverse_projection_matrix);
    float EnvZ = texture(dtex, xy).x;
    vec4 EnvPos = getPosFromUVDepth(vec3(xy, EnvZ), u_inverse_projection_matrix);
    float alpha = clamp((EnvPos.z - FragmentPos.z) * 0.3, 0., 1.);
    // TODO remove this later if possible when implementing GE
    alpha = mix(alpha, texture(tex, tc).a, billboard);
#else
    float alpha = 1.0;
#endif
    color = vec4(color.rgb * color.a * alpha, color.a * alpha);
    FragColor = color;
}
