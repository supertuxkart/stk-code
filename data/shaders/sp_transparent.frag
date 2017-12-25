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

uniform int fog_enabled;
uniform float custom_alpha;

in vec2 uv;
in vec4 color;
out vec4 o_diffuse_color;

void main()
{

#ifdef Use_Array_Texture
    vec4 diffusecolor = texture(tex_array, vec3(uv, array_0));
#else
    vec4 diffusecolor = texture(tex_layer_0, uv);
#endif

    vec4 finalcolor = vec4(0.);
    if (fog_enabled == 0)
    {
        finalcolor = diffusecolor;
        finalcolor.xyz *= color.xyz;
        finalcolor.a *= color.a;
    }
    else
    {
        diffusecolor.xyz *= color.xyz;
        diffusecolor.a *= color.a;
        vec3 tmp = vec3(gl_FragCoord.xy / u_screen, gl_FragCoord.z);
        tmp = 2. * tmp - 1.;
        vec4 xpos = vec4(tmp, 1.0);
        xpos = u_inverse_projection_matrix * xpos;
        xpos.xyz /= xpos.w;
        float dist = length(xpos.xyz);
        float fog = smoothstep(u_fog_data.x, u_fog_data.y, dist);
        fog = min(fog, u_fog_data.z);
        finalcolor = u_fog_color * fog + diffusecolor * (1. - fog);
    }
    o_diffuse_color = vec4(finalcolor.rgb * (finalcolor.a * (1.0 - custom_alpha)),
        finalcolor.a * (1.0 - custom_alpha));
}
