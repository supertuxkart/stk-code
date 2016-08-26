uniform sampler2D tex;
uniform float scale;

out vec4 FragColor;

#stk_include "utils/getCIEXYZ.frag"
#stk_include "utils/getRGBfromCIEXxy.frag"

void main()
{
    vec2 uv = gl_FragCoord.xy / (512. * scale);
    vec3 col = texture(tex, uv).xyz;
    vec3 Yxy = getCIEYxy(col);
    vec3 WhiteYxy = getCIEYxy(vec3(1.));

    Yxy.x = smoothstep(WhiteYxy.x, WhiteYxy.x * 4., Yxy.x);

    FragColor = vec4(max(vec3(0.), getRGBFromCIEXxy(Yxy)), 1.0);
}
