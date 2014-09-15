uniform sampler2D tex;
uniform float low;

out vec4 FragColor;

vec3 getCIEYxy(vec3 rgbColor);

void main()
{
    vec2 uv = gl_FragCoord.xy / 512;
    vec3 col = texture(tex, uv).xyz;
    float luma = getCIEYxy(col).x;

    col *= smoothstep(1., 10., luma);
    FragColor = max(vec4(col, 1.0), vec4(0.));
}
