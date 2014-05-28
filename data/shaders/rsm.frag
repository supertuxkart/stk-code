uniform sampler2D tex;

in vec2 uv;
in vec3 nor;
layout (location = 0) out vec3 RSMColor;
layout (location = 1) out vec3 RSMNormals;

void main()
{
    RSMColor = texture(tex, uv).xyz;
    RSMNormals = .5 * normalize(nor) + .5;
}
