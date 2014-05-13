uniform sampler2D Albedo;
uniform sampler2D caustictex;
uniform vec2 dir;
uniform vec2 dir2;

#if __VERSION__ >= 130
in vec2 uv;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif

vec3 getLightFactor(float specMapValue);

void main()
{
    vec4 color = texture(Albedo, uv);
    float caustic = texture(caustictex, uv + dir).x;
    float caustic2 = texture(caustictex, (uv.yx + dir2 * vec2(-0.6, 0.3)) * vec2(0.6)).x;

    vec3 LightFactor = getLightFactor(1.) + caustic * caustic2 * 10;
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
