uniform sampler2D Albedo;
uniform sampler2D Detail;

#if __VERSION__ >= 130
in vec2 uv;
in vec2 uv_bis;
out vec4 FragColor;
#else
varying vec2 uv;
varying vec2 uv_bis;
#define FragColor gl_FragColor
#endif

vec3 getLightFactor(float specMapValue);

void main(void)
{
    vec4 color = texture(Albedo, uv);
    vec4 detail = texture(Detail, uv_bis);
    color *= detail;
    vec3 LightFactor = getLightFactor(1. - color.a);
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
