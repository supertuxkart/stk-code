uniform sampler2D tex;

uniform float density;
uniform vec3 col;

out vec4 FragColor;


vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

void main()
{
    vec2 uv = 2. * gl_FragCoord.xy / screen;
    float z = texture(tex, uv).x;
    vec4 xpos = getPosFromUVDepth(vec3(uv, z), InverseProjectionMatrix);

    float dist = length(xpos.xyz);
    vec3 fog = col * (1. - exp(- density * dist));

    FragColor = vec4(fog, 1.);
}
