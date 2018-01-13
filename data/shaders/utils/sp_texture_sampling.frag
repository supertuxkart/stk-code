// Wrapper to allow easy sampling for material texture slots
uniform sampler2D tex_layer_0;
uniform sampler2D tex_layer_1;
uniform sampler2D tex_layer_2;
uniform sampler2D tex_layer_3;
uniform sampler2D tex_layer_4;
uniform sampler2D tex_layer_5;

vec4 sampleTextureSlot0(vec2 uv)
{
    return texture(tex_layer_0, uv);
}

vec4 sampleTextureSlot1(vec2 uv)
{
    return texture(tex_layer_1, uv);
}

vec4 sampleTextureSlot2(vec2 uv)
{
    return texture(tex_layer_2, uv);
}

vec4 sampleTextureSlot3(vec2 uv)
{
    return texture(tex_layer_3, uv);
}

vec4 sampleTextureSlot4(vec2 uv)
{
    return texture(tex_layer_4, uv);
}

vec4 sampleTextureSlot5(vec2 uv)
{
    return texture(tex_layer_5, uv);
}
