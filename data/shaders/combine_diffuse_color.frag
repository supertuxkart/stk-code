uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D ssao_tex;
uniform sampler2D normal_color;
uniform sampler2D diffuse_color;
#if defined(GL_ES) && defined(GL_FRAGMENT_PRECISION_HIGH)
uniform highp sampler2D depth_stencil;
#else
uniform sampler2D depth_stencil;
#endif
uniform sampler2D light_scatter;

uniform vec4 bg_color;

out vec4 o_final_color;

#stk_include "utils/getPosFromUVDepth.frag"

void main()
{
    vec2 tc = gl_FragCoord.xy / u_screen;
    vec4 diffuseMatColor = texture(diffuse_color, tc);

    // Polish map is stored in normal color framebuffer .z
    // Metallic map is stored in normal color framebuffer .w
    // Emit map is stored in diffuse color framebuffer.w
    float metallicMapValue = texture(normal_color, tc).w;
    float specMapValue = texture(normal_color, tc).z;
    float emitMapValue = diffuseMatColor.w;

    float ao = texture(ssao_tex, tc).x;
    vec3 DiffuseComponent = texture(diffuse_map, tc).xyz;
    vec3 SpecularComponent = texture(specular_map, tc).xyz;

    vec3 diffuse_color_for_mix = diffuseMatColor.xyz * 4.0;

    // FIXME enable this once the fallback shader is properly done!!!
    //vec3 metallicMatColor = mix(vec3(specMapValue), diffuse_color_for_mix, metallicMapValue);
    vec3 metallicMatColor = mix(vec3(0.04), diffuse_color_for_mix, metallicMapValue);
    // END FIXME
    
    vec3 tmp = DiffuseComponent * mix(diffuseMatColor.xyz, vec3(0.0), metallicMapValue) + (metallicMatColor * SpecularComponent);

    vec3 emitCol = diffuseMatColor.xyz + (diffuseMatColor.xyz * diffuseMatColor.xyz * emitMapValue * emitMapValue * 10.0);
    vec4 color_1 = vec4(tmp * ao + (emitMapValue * emitCol), 1.0);

    // Fog
    float depth = texture(depth_stencil, tc).x;
    vec4 xpos = getPosFromUVDepth(vec3(tc, depth), u_inverse_projection_matrix);
    float dist = length(xpos.xyz);
    // fog density
    float factor = (1.0 - exp(u_fog_data.w * dist));
    vec3 fog = u_fog_color.xyz * factor;

    // Additively blend the color by fog
    color_1 = color_1 + vec4(fog, factor);

    // For skybox blending later
    if (depth == 1.0)
    {
        color_1 = bg_color;
    }

    // Light scatter (alpha blend function: (GL_ONE, GL_ONE_MINUS_SRC_ALPHA))
    vec4 ls = texture(light_scatter, tc);
    vec4 color_2;
    color_2.r = ls.r + color_1.r * (1.0 - ls.a);
    color_2.g = ls.g + color_1.g * (1.0 - ls.a);
    color_2.b = ls.b + color_1.b * (1.0 - ls.a);
    color_2.a = ls.a + color_1.a * (1.0 - ls.a);
    o_final_color = color_2;
}
