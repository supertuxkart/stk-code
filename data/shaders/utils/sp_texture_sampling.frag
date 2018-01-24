// Wrapper to allow easy sampling for material texture layers
uniform sampler2D tex_layer_0;
uniform sampler2D tex_layer_1;
uniform sampler2D tex_layer_2;
uniform sampler2D tex_layer_3;
uniform sampler2D tex_layer_4;
uniform sampler2D tex_layer_5;

vec4 sampleTextureLayer0(vec2 uv)
{
    return texture(tex_layer_0, uv);
}

vec4 sampleTextureLayer1(vec2 uv)
{
    return texture(tex_layer_1, uv);
}

vec4 sampleTextureLayer2(vec2 uv)
{
    return texture(tex_layer_2, uv);
}

vec4 sampleTextureLayer3(vec2 uv)
{
    return texture(tex_layer_3, uv);
}

vec4 sampleTextureLayer4(vec2 uv)
{
    return texture(tex_layer_4, uv);
}

vec4 sampleTextureLayer5(vec2 uv)
{
    return texture(tex_layer_5, uv);
}
