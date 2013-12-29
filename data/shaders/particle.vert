#version 130
in vec3 position;
in float lf;
uniform mat4 matrix;

out float lifetime;

void main(void)
{
    gl_PointSize = 10.;
    gl_Position = matrix * vec4(position, 1.0);
	lifetime = lf;
}
