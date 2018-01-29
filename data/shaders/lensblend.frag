/*
Lens flare blend
based on bloomblend.frag
author: samuncle
*/
uniform sampler2D tex_128;
uniform sampler2D tex_256;
uniform sampler2D tex_512;

out vec4 FragColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    vec4 col = .125 * texture(tex_128, uv);
    col += .25 * texture(tex_256, uv);
    col += .5 * texture(tex_512, uv);
    
    // Blue color for lens flare
    /*col *= 0.5;
    float final = max(col.r,max(col.g,col.b));
    final = final * 2;
    vec3 blue = vec3(final * 0.1, final * 0.2, final);*/
    
    FragColor = vec4(col.rgb, 1.);
}
