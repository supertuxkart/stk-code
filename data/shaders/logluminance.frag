uniform sampler2D tex;

in vec2 uv;
out vec4 FragColor;

float delta = .0001;

void main()
{
    vec3 weight = vec3(0.2125f, 0.7154f, 0.0721f);
    vec3 col = texture(tex, uv).xyz;
    float luma = dot(col, weight);
    FragColor = vec4(log(luma + delta));
}