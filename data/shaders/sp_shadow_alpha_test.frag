#ifdef Use_Bindless_Texture
flat in sampler2D tex_layer_0;
#else
// spm layer 1 texture
uniform sampler2D tex_layer_0;
#endif

#ifdef Use_Array_Texture
uniform sampler2DArray tex_array;
flat in float array_0;
#endif

in vec2 uv;
out vec4 o_frag_color;

void main(void)
{

#ifdef Use_Array_Texture
    vec4 col = texture(tex_array, vec3(uv, array_0));
#else
    vec4 col = texture(tex_layer_0, uv);
#endif

    if (col.a < 0.5)
    {
        discard;
    }
    o_frag_color = vec4(1.0);
}
