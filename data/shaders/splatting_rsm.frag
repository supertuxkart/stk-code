uniform sampler2D tex_layout;
uniform sampler2D tex_detail0;
uniform sampler2D tex_detail1;
uniform sampler2D tex_detail2;
uniform sampler2D tex_detail3;

in vec2 uv;
in vec2 uv_bis;
in vec3 nor;
layout (location = 0) out vec3 RSMColor;
layout (location = 1) out vec3 RSMNormals;

void main()
{
    vec4 splatting = texture(tex_layout, uv_bis);
    vec4 detail0 = texture(tex_detail0, uv);
    vec4 detail1 = texture(tex_detail1, uv);
    vec4 detail2 = texture(tex_detail2, uv);
    vec4 detail3 = texture(tex_detail3, uv);

    vec4 splatted = splatting.r * detail0 +
        splatting.g * detail1 +
        splatting.b * detail2 +
        max(0., (1.0 - splatting.r - splatting.g - splatting.b)) * detail3;
    RSMColor = splatted.rgb;
    RSMNormals = .5 * normalize(nor) + .5;
}
