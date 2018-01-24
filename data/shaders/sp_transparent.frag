uniform int fog_enabled;
uniform float custom_alpha;

in vec2 uv;
in vec4 color;
out vec4 o_diffuse_color;

#stk_include "utils/sp_texture_sampling.frag"

void main()
{
    vec4 diffusecolor = sampleTextureLayer0(uv);
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
