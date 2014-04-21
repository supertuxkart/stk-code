uniform sampler2D tex;

in vec2 uv;
out vec4 FragColor;

float delta = .1;

void main()
{
    vec3 weight = vec3(0.2125f, 0.7154f, 0.0721f);
    vec3 col = texture(tex, uv).xyz;
    // TODO: Investigate why color buffer has negative value sometimes
    float luma = max(dot(col, weight), 0.);
    FragColor = vec4(log(luma + delta));
}