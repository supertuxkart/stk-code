uniform sampler2D Albedo;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif

vec3 getLightFactor(float specMapValue);

void main(void)
{
    vec4 color = texture(Albedo, uv);
    vec3 LightFactor = getLightFactor(1. - color.a);
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
