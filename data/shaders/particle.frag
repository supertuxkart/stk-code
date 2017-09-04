uniform sampler2D tex;
uniform sampler2D dtex;
uniform mat4 invproj;

in float lf;
in vec2 tc;
in vec4 pc;
out vec4 FragColor;

#stk_include "utils/getPosFromUVDepth.frag"

void main(void)
{
    vec2 xy = gl_FragCoord.xy / screen;
    float FragZ = gl_FragCoord.z;
    vec4 FragmentPos = getPosFromUVDepth(vec3(xy, FragZ), InverseProjectionMatrix);
    float EnvZ = texture(dtex, xy).x;
    vec4 EnvPos = getPosFromUVDepth(vec3(xy, EnvZ), InverseProjectionMatrix);
    float alpha = clamp((EnvPos.z - FragmentPos.z) * 0.3, 0., 1.);
    FragColor = texture(tex, tc) * pc * alpha;
}
