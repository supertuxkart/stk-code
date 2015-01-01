uniform samplerCube tex;

out vec4 FragColor;

void main(void)
{
    vec3 eyedir = gl_FragCoord.xyz / vec3(screen, 1.);
    eyedir = 2.0 * eyedir - 1.0;
    vec4 tmp = (InverseProjectionMatrix * vec4(eyedir, 1.));
    tmp /= tmp.w;
    eyedir = (InverseViewMatrix * vec4(tmp.xyz, 0.)).xyz;
    vec4 color = texture(tex, eyedir);
    FragColor = vec4(color.xyz, 1.);
}
