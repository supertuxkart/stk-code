uniform sampler2D tex;

in vec2 uv;
in vec3 nor;
in vec4 color;
layout (location = 0) out vec3 RSMColor;
layout (location = 1) out vec3 RSMNormals;

void main()
{
    if (texture(tex, uv).a < .5) discard;
    RSMColor = texture(tex, uv).xyz * color.rgb;
    RSMNormals = .5 * normalize(nor) + .5;
}
