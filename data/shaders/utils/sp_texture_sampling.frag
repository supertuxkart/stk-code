// Wrapper to allow easy sampling for material texture layers
uniform sampler2D tex_layer_0;
uniform sampler2D tex_layer_1;
uniform sampler2D tex_layer_2;
uniform sampler2D tex_layer_3;
uniform sampler2D tex_layer_4;
uniform sampler2D tex_layer_5;

#define HIGH_SAMPLING   4.0
#define MEDIUM_SAMPLING 2.0
#define LOW_SAMPLING    1.0

vec4 sampleTextureLayer0(vec2 uv)
{
    return texture(tex_layer_0, uv);
}

vec4 multi_sampleTextureLayer0(vec2 uv, float distance)
{

    vec4 l_col = sampleTextureLayer0(uv * LOW_SAMPLING);
    vec4 m_col = sampleTextureLayer0(uv * MEDIUM_SAMPLING);
    vec4 h_col = sampleTextureLayer0(uv * HIGH_SAMPLING);

    /* debug
    l_col = vec4(1.0, 0.0, 0.0, 1.0);
    m_col = vec4(0.0, 1.0, 0.0, 1.0);
    h_col = vec4(0.0, 0.0, 1.0, 1.0);*/

    // From Low to medium
    float factor = distance * 0.02;
    factor = pow(factor, 2.5);
    factor = clamp(factor, 0.0, 1.0);
    vec4 f_col = mix(m_col, l_col, factor);
    
    // From medium to high
    factor = distance * 0.1;
    factor = pow(factor, 2.5);
    factor = clamp(factor, 0.0, 1.0);

    f_col = mix(h_col, f_col, factor);

    return f_col;
}

vec4 sampleTextureLayer1(vec2 uv)
{
    return texture(tex_layer_1, uv);
}

vec4 sampleTextureLayer2(vec2 uv)
{
    return texture(tex_layer_2, uv);
}

vec4 multi_sampleTextureLayer2(vec2 uv, float distance)
{

    vec4 l_col = sampleTextureLayer2(uv * LOW_SAMPLING);
    vec4 m_col = sampleTextureLayer2(uv * MEDIUM_SAMPLING);
    vec4 h_col = sampleTextureLayer2(uv * HIGH_SAMPLING);

    // From Low to medium
    float factor = distance * 0.02;
    factor = pow(factor, 2.5);
    factor = clamp(factor, 0.0, 1.0);
    vec4 f_col = mix(m_col, l_col, factor);
    
    // From medium to high
    factor = distance * 0.1;
    factor = pow(factor, 2.5);
    factor = clamp(factor, 0.0, 1.0);

    f_col = mix(h_col, f_col, factor);

    return f_col;
}

vec4 sampleTextureLayer3(vec2 uv)
{
    return texture(tex_layer_3, uv);
}

vec4 multi_sampleTextureLayer3(vec2 uv, float distance)
{

    vec4 l_col = sampleTextureLayer3(uv * LOW_SAMPLING);
    vec4 m_col = sampleTextureLayer3(uv * MEDIUM_SAMPLING);
    vec4 h_col = sampleTextureLayer3(uv * HIGH_SAMPLING);

    // From Low to medium
    float factor = distance * 0.02;
    factor = pow(factor, 2.5);
    factor = clamp(factor, 0.0, 1.0);
    vec4 f_col = mix(m_col, l_col, factor);
    
    // From medium to high
    factor = distance * 0.1;
    factor = pow(factor, 2.5);
    factor = clamp(factor, 0.0, 1.0);

    f_col = mix(h_col, f_col, factor);

    return f_col;
}

vec4 sampleTextureLayer4(vec2 uv)
{
    return texture(tex_layer_4, uv);
}

vec4 multi_sampleTextureLayer4(vec2 uv, float distance)
{

    vec4 l_col = sampleTextureLayer4(uv * LOW_SAMPLING);
    vec4 m_col = sampleTextureLayer4(uv * MEDIUM_SAMPLING);
    vec4 h_col = sampleTextureLayer4(uv * HIGH_SAMPLING);

    // From Low to medium
    float factor = distance * 0.02;
    factor = pow(factor, 2.5);
    factor = clamp(factor, 0.0, 1.0);
    vec4 f_col = mix(m_col, l_col, factor);
    
    // From medium to high
    factor = distance * 0.1;
    factor = pow(factor, 2.5);
    factor = clamp(factor, 0.0, 1.0);

    f_col = mix(h_col, f_col, factor);

    return f_col;
}

vec4 sampleTextureLayer5(vec2 uv)
{
    return texture(tex_layer_5, uv);
}
