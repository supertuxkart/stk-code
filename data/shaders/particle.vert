#version 130
uniform mat4 matrix;

in vec2 quadcorner;
in vec2 texcoord;
in vec3 position;
in float lifetime;

out float lf;
out vec2 tc;

void main(void)
{
	tc = texcoord;
	lf = lifetime;
    gl_Position = matrix * vec4(vec3(quadcorner, 0.) + position, 1.0);
}
