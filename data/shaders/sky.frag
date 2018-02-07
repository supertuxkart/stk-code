uniform samplerCube tex;

out vec4 FragColor;

void main(void)
{
    vec3 eyedir = vec3(mod(gl_FragCoord.xy, u_screen) / u_screen, 1.);
    eyedir = 2.0 * eyedir - 1.0;
    vec4 tmp = (u_inverse_projection_matrix * vec4(eyedir, 1.));
    tmp /= tmp.w;
    eyedir = (u_inverse_view_matrix * vec4(tmp.xyz, 0.)).xyz;
    vec4 color = texture(tex, eyedir);
    FragColor = vec4(color.xyz, 1.);
}
