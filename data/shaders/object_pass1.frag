#version 130
noperspective in vec3 nor;
out vec3 Normal;

void main(void)
{
    Normal = 0.5 * normalize(nor) + 0.5;
}
