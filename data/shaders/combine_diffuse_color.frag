uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D ssao_tex;
uniform sampler2D gloss_map;
uniform sampler2D diffuse_color;
uniform sampler2D depth_stencil;

out vec4 o_final_color;

#stk_include "utils/getPosFromUVDepth.frag"

void main()
{
    vec2 tc = gl_FragCoord.xy / u_screen;
    vec4 diffuseMatColor = texture(diffuse_color, tc);

    // Gloss map here is stored in red and green for metallic and emit map
    // Real gloss channel is stored in normal and depth framebuffer .z
    float metallicMapValue = texture(gloss_map, tc).x;
    float emitMapValue = texture(gloss_map, tc).y;

    float ao = texture(ssao_tex, tc).x;
    vec3 DiffuseComponent = texture(diffuse_map, tc).xyz;
    vec3 SpecularComponent = texture(specular_map, tc).xyz;

    vec3 metallicMatColor = mix(vec3(0.04), diffuseMatColor.xyz * 4, metallicMapValue);
    vec3 tmp = DiffuseComponent * mix(diffuseMatColor.xyz, vec3(0.0), metallicMapValue) + (metallicMatColor * SpecularComponent);

    vec3 emitCol = diffuseMatColor.xyz * diffuseMatColor.xyz * diffuseMatColor.xyz * 15.;
    vec4 color_1 = vec4(tmp * ao + (emitMapValue * emitCol), diffuseMatColor.a);

    // Fog
    float z = texture(depth_stencil, tc).x;
    vec4 xpos = getPosFromUVDepth(vec3(tc, z), u_inverse_projection_matrix);
    float dist = length(xpos.xyz);
    // fog density
    float factor = (1.0 - exp(u_fog_data.w * dist));
    vec3 fog = u_fog_color.xyz * factor;

    // Additively blend the color by fog
    o_final_color = color_1 + vec4(fog, factor);
}
