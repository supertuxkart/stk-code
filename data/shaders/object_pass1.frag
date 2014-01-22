#version 130
noperspective in vec3 nor;
out vec4 FragColor;

void main(void)
{
    FragColor = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
}
