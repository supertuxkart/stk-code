uniform samplerCube tex;

#if __VERSION__ >= 130
out vec4 FragColor;
#else
#define FragColor gl_FragColor
#endif


void main(void)
{
    vec3 eyedir = gl_FragCoord.xyz / vec3(screen, 1.);
    eyedir = 2.0 * eyedir - 1.0;
	vec4 tmp = (InverseViewMatrix * InverseProjectionMatrix * vec4(eyedir, 1.));
	eyedir = tmp.xyz / tmp.w;
    vec4 color = texture(tex, eyedir);
    FragColor = vec4(color.xyz, 1.);
}
