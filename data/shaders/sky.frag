#version 330
uniform samplerCube tex;
uniform mat4 InvProjView;
uniform vec2 screen;

in vec2 uv;
out vec4 FragColor;

void main(void)
{
    vec3 eyedir = gl_FragCoord.xyz / vec3(screen, 1.);
    eyedir = 2.0 * eyedir - 1.0;
	vec4 tmp = (InvProjView * vec4(eyedir, 1.));
	eyedir = tmp.xyz / tmp.w;
    vec4 color = texture(tex, eyedir);
    FragColor = vec4(color.xyz, 1.);
}
