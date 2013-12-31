#version 130
uniform mat4 matrix;

in vec3 position;
in float lifetime;

out float lf;

void main(void)
{
	lf = lifetime;
    gl_Position = matrix * vec4(position, 1.0);
    gl_PointSize = 300. / gl_Position.w;
}
