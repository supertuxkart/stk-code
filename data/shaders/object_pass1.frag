#version 130
noperspective in vec3 nor;

void main(void)
{
    gl_FragColor = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
}
