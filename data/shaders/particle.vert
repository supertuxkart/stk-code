#version 130
uniform mat4 matrix;
in vec3 position;

void main(void)
{
    gl_PointSize = 50.;
    gl_Position = matrix * vec4(position, 1.0);
}
