
in vec4 color;
out vec4 FragColor;

vec3 getLightFactor(float specMapValue);

void main(void)
{
    vec3 LightFactor = getLightFactor(1.);
    FragColor = vec4(color.xyz * LightFactor, 1.);
}
