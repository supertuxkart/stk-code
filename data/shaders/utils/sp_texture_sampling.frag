// Wrapper to allow easy sampling for material texture slots
#if defined(Use_Array_Texture)
flat in float array_0;
flat in float array_1;
flat in float array_2;
flat in float array_3;
flat in float array_4;
flat in float array_5;
uniform sampler2DArray tex_array;
#elif defined(Use_Bindless_Texture)
flat in sampler2D tex_layer_0;
flat in sampler2D tex_layer_1;
flat in sampler2D tex_layer_2;
flat in sampler2D tex_layer_3;
flat in sampler2D tex_layer_4;
flat in sampler2D tex_layer_5;
#else
uniform sampler2D tex_layer_0;
uniform sampler2D tex_layer_1;
uniform sampler2D tex_layer_2;
uniform sampler2D tex_layer_3;
uniform sampler2D tex_layer_4;
uniform sampler2D tex_layer_5;
#endif

vec4 sampleTextureSlot0(vec2 uv)
{
#ifdef Use_Array_Texture
    return texture(tex_array, vec3(uv, array_0));
#else
    return texture(tex_layer_0, uv);
#endif
}

vec4 sampleTextureSlot1(vec2 uv)
{
#ifdef Use_Array_Texture
    return texture(tex_array, vec3(uv, array_1));
#else
    return texture(tex_layer_1, uv);
#endif
}

vec4 sampleTextureSlot2(vec2 uv)
{
#ifdef Use_Array_Texture
    return texture(tex_array, vec3(uv, array_2));
#else
    return texture(tex_layer_2, uv);
#endif
}

vec4 sampleTextureSlot3(vec2 uv)
{
#ifdef Use_Array_Texture
    return texture(tex_array, vec3(uv, array_3));
#else
    return texture(tex_layer_3, uv);
#endif
}

vec4 sampleTextureSlot4(vec2 uv)
{
#ifdef Use_Array_Texture
    return texture(tex_array, vec3(uv, array_4));
#else
    return texture(tex_layer_4, uv);
#endif
}

vec4 sampleTextureSlot5(vec2 uv)
{
#ifdef Use_Array_Texture
    return texture(tex_array, vec3(uv, array_5));
#else
    return texture(tex_layer_5, uv);
#endif
}
